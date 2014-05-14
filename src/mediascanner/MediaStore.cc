/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * Authors:
 *    Jussi Pakkanen <jussi.pakkanen@canonical.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <mutex>

#include <glib.h>
#include <sqlite3.h>

#include "mozilla/fts3_tokenizer.h"
#include "MediaStore.hh"
#include "MediaFile.hh"
#include "MediaFileBuilder.hh"
#include "Album.hh"
#include "internal/sqliteutils.hh"
#include "internal/utils.hh"

using namespace std;

namespace mediascanner {

// Increment this whenever changing db schema.
// It will cause dbstore to rebuild its tables.
static const int schemaVersion = 5;

struct MediaStorePrivate {
    sqlite3 *db;
    // https://www.sqlite.org/cvstrac/wiki?p=DatabaseIsLocked
    // http://sqlite.com/faq.html#q6
    std::mutex dbMutex;

    void insert(const MediaFile &m) const;
    void remove(const std::string &fname) const;
    MediaFile lookup(const std::string &filename) const;
    std::vector<MediaFile> query(const std::string &q, MediaType type, int limit=-1) const;
    std::vector<Album> queryAlbums(const std::string &core_term, int limit=-1) const;
    std::vector<MediaFile> getAlbumSongs(const Album& album) const;
    std::string getETag(const std::string &filename) const;

    size_t size() const;
    void pruneDeleted();
    void archiveItems(const std::string &prefix);
    void restoreItems(const std::string &prefix);
};

extern "C" void sqlite3Fts3PorterTokenizerModule(
    sqlite3_tokenizer_module const**ppModule);

static void register_tokenizer(sqlite3 *db) {
    Statement query(db, "SELECT fts3_tokenizer(?, ?)");

    query.bind(1, "mozporter");
    const sqlite3_tokenizer_module *p = NULL;
    sqlite3Fts3PorterTokenizerModule(&p);
    query.bind(2, &p, sizeof(p));

    query.step();
}

/* ranking function adapted from http://sqlite.org/fts3.html#appendix_a */
static void rankfunc(sqlite3_context *pCtx, int nVal, sqlite3_value **apVal) {
    const int32_t *aMatchinfo;      /* Return value of matchinfo() */
    int32_t nCol;                   /* Number of columns in the table */
    int32_t nPhrase;                /* Number of phrases in the query */
    int32_t iPhrase;                /* Current phrase */
    double score = 0.0;             /* Value to return */

    /* Check that the number of arguments passed to this function is correct.
    ** If not, jump to wrong_number_args. Set aMatchinfo to point to the array
    ** of unsigned integer values returned by FTS function matchinfo. Set
    ** nPhrase to contain the number of reportable phrases in the users full-text
    ** query, and nCol to the number of columns in the table.
    */
    if( nVal<1 ) goto wrong_number_args;
    aMatchinfo = static_cast<const int32_t*>(sqlite3_value_blob(apVal[0]));
    nPhrase = aMatchinfo[0];
    nCol = aMatchinfo[1];
    if( nVal!=(1+nCol) ) goto wrong_number_args;

    /* Iterate through each phrase in the users query. */
    for(iPhrase=0; iPhrase<nPhrase; iPhrase++){
        int32_t iCol;                     /* Current column */

        /* Now iterate through each column in the users query. For each column,
        ** increment the relevancy score by:
        **
        **   (<hit count> / <global hit count>) * <column weight>
        **
        ** aPhraseinfo[] points to the start of the data for phrase iPhrase. So
        ** the hit count and global hit counts for each column are found in 
        ** aPhraseinfo[iCol*3] and aPhraseinfo[iCol*3+1], respectively.
        */
        const int32_t *aPhraseinfo = &aMatchinfo[2 + iPhrase*nCol*3];
        for(iCol=0; iCol<nCol; iCol++){
            int32_t nHitCount = aPhraseinfo[3*iCol];
            int32_t nGlobalHitCount = aPhraseinfo[3*iCol+1];
            double weight = sqlite3_value_double(apVal[iCol+1]);
            if( nHitCount>0 ){
                score += ((double)nHitCount / (double)nGlobalHitCount) * weight;
            }
        }
    }

    sqlite3_result_double(pCtx, score);
    return;

    /* Jump here if the wrong number of arguments are passed to this function */
wrong_number_args:
    sqlite3_result_error(pCtx, "wrong number of arguments to function rank()", -1);
}

static void register_functions(sqlite3 *db) {
    if (sqlite3_create_function(db, "rank", -1, SQLITE_ANY, NULL,
                                rankfunc, NULL, NULL) != SQLITE_OK) {
        throw runtime_error(sqlite3_errmsg(db));
    }
}

static void execute_sql(sqlite3 *db, const string &cmd) {
    char *errmsg = nullptr;
    if(sqlite3_exec(db, cmd.c_str(), nullptr, nullptr, &errmsg) != SQLITE_OK) {
        throw runtime_error(errmsg);
    }
}

static int getSchemaVersion(sqlite3 *db) {
    int version = -1;
    try {
        Statement select(db, "SELECT version FROM schemaVersion");
        if (select.step())
            version = select.getInt(0);
    } catch (const exception &e) {
        /* schemaVersion table might not exist */
    }
    return version;
}

void deleteTables(sqlite3 *db) {
    string deleteCmd(R"(
DROP TABLE IF EXISTS media;
DROP TABLE IF EXISTS media_fts;
DROP TABLE IF EXISTS media_attic;
DROP TABLE IF EXISTS schemaVersion;
)");
    execute_sql(db, deleteCmd);
}

void createTables(sqlite3 *db) {
    string schema(R"(
CREATE TABLE schemaVersion (version INTEGER);

CREATE TABLE media (
    filename TEXT PRIMARY KEY NOT NULL,
    content_type TEXT,
    etag TEXT,
    title TEXT,
    date TEXT,
    artist TEXT,          -- Only relevant to audio
    album TEXT,           -- Only relevant to audio
    album_artist TEXT,    -- Only relevant to audio
    genre TEXT,           -- Only relevant to audio
    disc_number INTEGER,  -- Only relevant to audio
    track_number INTEGER, -- Only relevant to audio
    duration INTEGER,
    type INTEGER   -- 0=Audio, 1=Video
);

CREATE INDEX media_album_album_artist_idx ON media(album, album_artist);

CREATE TABLE media_attic (
    filename TEXT PRIMARY KEY NOT NULL,
    content_type TEXT,
    etag TEXT,
    title TEXT,
    date TEXT,
    artist TEXT,          -- Only relevant to audio
    album TEXT,           -- Only relevant to audio
    album_artist TEXT,    -- Only relevant to audio
    genre TEXT,           -- Only relevant to audio
    disc_number INTEGER,  -- Only relevant to audio
    track_number INTEGER, -- Only relevant to audio
    duration INTEGER,
    type INTEGER   -- 0=Audio, 1=Video
);

CREATE VIRTUAL TABLE media_fts 
USING fts4(content='media', title, artist, album, tokenize=mozporter);

CREATE TRIGGER media_bu BEFORE UPDATE ON media BEGIN
  DELETE FROM media_fts WHERE docid=old.rowid;
END;

CREATE TRIGGER media_au AFTER UPDATE ON media BEGIN
  INSERT INTO media_fts(docid, title, artist, album) VALUES (new.rowid, new.title, new.artist, new.album);
END;

CREATE TRIGGER media_bd BEFORE DELETE ON media BEGIN
  DELETE FROM media_fts WHERE docid=old.rowid;
END;

CREATE TRIGGER media_ai AFTER INSERT ON media BEGIN
  INSERT INTO media_fts(docid, title, artist, album) VALUES (new.rowid, new.title, new.artist, new.album);
END;
)");
    execute_sql(db, schema);

    Statement version(db, "INSERT INTO schemaVersion (version) VALUES (?)");
    version.bind(1, schemaVersion);
    version.step();
}

static std::string get_default_database() {
    std::string cachedir;

    char *env_cachedir = getenv("MEDIASCANNER_CACHEDIR");
    if (env_cachedir) {
        cachedir = env_cachedir;
    } else {
        cachedir = g_get_user_cache_dir();
        cachedir += "/mediascanner-2.0";
    }
    if (g_mkdir_with_parents(cachedir.c_str(), S_IRWXU) < 0) {
        std::string msg("Could not create cache dir: ");
        msg += strerror(errno);
        throw runtime_error(msg);
    }
    return cachedir + "/mediastore.db";
}

MediaStore::MediaStore(OpenType access, const std::string &retireprefix)
    : MediaStore(get_default_database(), access, retireprefix)
{
}

MediaStore::MediaStore(const std::string &filename, OpenType access, const std::string &retireprefix) {
    int sqliteFlags = access == MS_READ_WRITE ? SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE : SQLITE_OPEN_READONLY;
    p = new MediaStorePrivate();
    if(sqlite3_open_v2(filename.c_str(), &p->db, sqliteFlags, nullptr) != SQLITE_OK) {
        throw runtime_error(sqlite3_errmsg(p->db));
    }
    register_tokenizer(p->db);
    register_functions(p->db);
    int detectedSchemaVersion = getSchemaVersion(p->db);
    if(access == MS_READ_WRITE) {
        if(detectedSchemaVersion != schemaVersion) {
            deleteTables(p->db);
            createTables(p->db);
        }
        if(!retireprefix.empty())
            archiveItems(retireprefix);
    } else {
        if(detectedSchemaVersion != schemaVersion) {
            throw runtime_error("Tried to open a db with an unsupported schema version.");
        }
    }
}

MediaStore::~MediaStore() {
    sqlite3_close(p->db);
    delete p;
}

size_t MediaStorePrivate::size() const {
    Statement count(db, "SELECT COUNT(*) FROM media");
    count.step();
    return count.getInt(0);
}

void MediaStorePrivate::insert(const MediaFile &m) const {
    Statement query(db, "INSERT OR REPLACE INTO media (filename, content_type, etag, title, date, artist, album, album_artist, genre, disc_number, track_number, duration, type)  VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    query.bind(1, m.getFileName());
    query.bind(2, m.getContentType());
    query.bind(3, m.getETag());
    query.bind(4, m.getTitle());
    query.bind(5, m.getDate());
    query.bind(6, m.getAuthor());
    query.bind(7, m.getAlbum());
    query.bind(8, m.getAlbumArtist());
    query.bind(9, m.getGenre());
    query.bind(10, m.getDiscNumber());
    query.bind(11, m.getTrackNumber());
    query.bind(12, m.getDuration());
    query.bind(13, (int)m.getType());
    query.step();

    const char *typestr = m.getType() == AudioMedia ? "song" : "video";
    printf("Added %s to backing store: %s\n", typestr, m.getFileName().c_str());
    printf(" author   : '%s'\n", m.getAuthor().c_str());
    printf(" title    : %s\n", m.getTitle().c_str());
    printf(" album    : '%s'\n", m.getAlbum().c_str());
    printf(" duration : %d\n", m.getDuration());
}

void MediaStorePrivate::remove(const string &fname) const {
    Statement del(db, "DELETE FROM media WHERE filename = ?");
    del.bind(1, fname);
    del.step();
}

static MediaFile make_media(Statement &query) {
    return MediaFileBuilder(query.getText(0))
        .setContentType(query.getText(1))
        .setETag(query.getText(2))
        .setTitle(query.getText(3))
        .setDate(query.getText(4))
        .setAuthor(query.getText(5))
        .setAlbum(query.getText(6))
        .setAlbumArtist(query.getText(7))
        .setGenre(query.getText(8))
        .setDiscNumber(query.getInt(9))
        .setTrackNumber(query.getInt(10))
        .setDuration(query.getInt(11))
        .setType((MediaType)query.getInt(12));
}

static vector<MediaFile> collect_media(Statement &query) {
    vector<MediaFile> result;
    while (query.step()) {
        result.push_back(make_media(query));
    }
    return result;
}

MediaFile MediaStorePrivate::lookup(const std::string &filename) const {
    Statement query(db, R"(
SELECT filename, content_type, etag, title, date, artist, album, album_artist, genre, disc_number, track_number, duration, type
  FROM media
  WHERE filename = ?
)");
    query.bind(1, filename);
    if (!query.step()) {
        throw runtime_error("Could not find media " + filename);
    }
    return make_media(query);
}

vector<MediaFile> MediaStorePrivate::query(const std::string &core_term, MediaType type, int limit) const {
    if (core_term == "") {
        Statement query(db, R"(
SELECT filename, content_type, etag, title, date, artist, album, album_artist, genre, disc_number, track_number, duration, type
  FROM media
  WHERE type == ?
  LIMIT ?
)");
        query.bind(1, (int)type);
        query.bind(2, limit);
        return collect_media(query);
    } else {
        Statement query(db, R"(
SELECT filename, content_type, etag, title, date, artist, album, album_artist, genre, disc_number, track_number, duration, type
  FROM media JOIN (
    SELECT docid, rank(matchinfo(media_fts), 1.0, 0.5, 0.75) AS rank
      FROM media_fts WHERE media_fts MATCH ?
    ) AS ranktable ON (media.rowid = ranktable.docid)
  WHERE type == ?
  ORDER BY ranktable.rank DESC
  LIMIT ?
)");
        query.bind(1, core_term + "*");
        query.bind(2, (int)type);
        query.bind(3, limit);
        return collect_media(query);
    }
}

static Album make_album(Statement &query) {
    const string album = query.getText(0);
    const string album_artist = query.getText(1);
    return Album(album, album_artist);
}

static vector<Album> collect_albums(Statement &query) {
    vector<Album> result;
    while (query.step()) {
        result.push_back(make_album(query));
    }
    return result;
}

vector<Album> MediaStorePrivate::queryAlbums(const std::string &core_term, int limit) const {
    if (core_term == "") {
        Statement query(db, R"(
SELECT album, album_artist FROM media
WHERE type = ? AND album <> ''
GROUP BY album, album_artist
LIMIT ?
)");
        query.bind(1, (int)AudioMedia);
        query.bind(2, limit);
        return collect_albums(query);
    } else {
        Statement query(db, R"(
SELECT album, album_artist FROM media
WHERE rowid IN (SELECT docid FROM media_fts WHERE media_fts MATCH ?)
AND type == ? AND album <> ''
GROUP BY album, album_artist
LIMIT ?
)");
        query.bind(1, core_term + "*");
        query.bind(2, (int)AudioMedia);
        query.bind(3, limit);
        return collect_albums(query);
    }
}

vector<MediaFile> MediaStorePrivate::getAlbumSongs(const Album& album) const {
    Statement query(db, R"(
SELECT filename, content_type, etag, title, date, artist, album, album_artist, genre, disc_number, track_number, duration, type FROM media
WHERE album = ? AND album_artist = ? AND type = ?
ORDER BY disc_number, track_number
)");
    query.bind(1, album.getTitle());
    query.bind(2, album.getArtist());
    query.bind(3, (int)AudioMedia);
    return collect_media(query);
}

std::string MediaStorePrivate::getETag(const std::string &filename) const {
    Statement query(db, R"(
SELECT etag FROM media WHERE filename = ?
)");
    query.bind(1, filename);
    if (query.step()) {
        return query.getText(0);
    } else {
        return "";
    }
}

std::vector<MediaFile> MediaStore::listSongs(const std::string& artist, const std::string& album, const std::string& album_artist, int limit) const {
    std::string qs(R"(
SELECT filename, content_type, etag, title, date, artist, album, album_artist, genre, disc_number, track_number, duration, type
  FROM media
  WHERE type = ?
)");
    if (!artist.empty()) {
        qs += " AND artist = ?";
    }
    if (!album.empty()) {
        qs += " AND album = ?";
    }
    if (!album_artist.empty()) {
        qs += " AND album_artist = ?";
    }
    qs += R"(
ORDER BY album_artist, album, disc_number, track_number, title
LIMIT ?
)";
    Statement query(p->db, qs.c_str());
    int param = 1;
    query.bind(param++, (int)AudioMedia);
    if (!artist.empty()) {
        query.bind(param++, artist);
    }
    if (!album.empty()) {
        query.bind(param++, album);
    }
    if (!album_artist.empty()) {
        query.bind(param++, album_artist);
    }
    query.bind(param++, limit);

    return collect_media(query);
}

std::vector<Album> MediaStore::listAlbums(const std::string& artist, const std::string& album_artist, int limit) const {
    std::string qs(R"(
SELECT album, album_artist FROM media
  WHERE type = ?
)");
    if (!artist.empty()) {
        qs += " AND artist = ?";
    }
    if (!album_artist.empty()) {
        qs += " AND album_artist = ?";
    }
    qs += R"(
GROUP BY album, album_artist
ORDER BY album_artist, album
LIMIT ?
)";
    Statement query(p->db, qs.c_str());
    int param = 1;
    query.bind(param++, (int)AudioMedia);
    if (!artist.empty()) {
        query.bind(param++, artist);
    }
    if (!album_artist.empty()) {
        query.bind(param++, album_artist);
    }
    query.bind(param++, limit);

    return collect_albums(query);
}

vector<std::string> MediaStore::listArtists(bool album_artists, int limit) {
    const char *qs;

    if (album_artists) {
        qs = R"(
SELECT album_artist FROM media
  GROUP BY album_artist
  ORDER BY album_artist
  LIMIT ?
)";
    } else {
        qs = R"(
SELECT artist FROM media
  GROUP BY artist
  ORDER BY artist
  LIMIT ?
)";
    }
    Statement query(p->db, qs);
    query.bind(1, limit);

    vector<string> artists;
    while (query.step()) {
        artists.push_back(query.getText(0));
    }
    return artists;
}

void MediaStorePrivate::pruneDeleted() {
    vector<string> deleted;
    Statement query(db, "SELECT filename FROM media");
    while (query.step()) {
        const string filename = query.getText(0);
        if (access(filename.c_str(), F_OK) != 0) {
            deleted.push_back(filename);
        }
    }
    query.finalize();
    printf("%d files deleted from disk.\n", (int)deleted.size());
    for(const auto &i : deleted) {
        remove(i);
    }
}

void MediaStorePrivate::archiveItems(const std::string &prefix) {
    const char *templ = R"(BEGIN TRANSACTION;
INSERT INTO media_attic SELECT * FROM media WHERE filename LIKE %s;
DELETE FROM media WHERE filename LIKE %s;
COMMIT;
)";
    string cond = sqlQuote(prefix + "%");
    const size_t bufsize = 1024;
    char cmd[bufsize];
    snprintf(cmd, bufsize, templ, cond.c_str(), cond.c_str());
    char *errmsg;
    if(sqlite3_exec(db, cmd, nullptr, nullptr, &errmsg) != SQLITE_OK) {
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        throw runtime_error(errmsg);
    }
}

void MediaStorePrivate::restoreItems(const std::string &prefix) {
    const char *templ = R"(BEGIN TRANSACTION;
INSERT INTO media SELECT * FROM media_attic WHERE filename LIKE %s;
DELETE FROM media_attic WHERE filename LIKE %s;
COMMIT;
)";
    string cond = sqlQuote(prefix + "%");
    const size_t bufsize = 1024;
    char cmd[bufsize];
    snprintf(cmd, bufsize, templ, cond.c_str(), cond.c_str());
    char *errmsg;
    if(sqlite3_exec(db, cmd, nullptr, nullptr, &errmsg) != SQLITE_OK) {
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        throw runtime_error(errmsg);
    }

}

void MediaStore::insert(const MediaFile &m) const {
    std::lock_guard<std::mutex> lock(p->dbMutex);
    p->insert(m);
}

void MediaStore::remove(const std::string &fname) const {
    std::lock_guard<std::mutex> lock(p->dbMutex);
    p->remove(fname);
}

MediaFile MediaStore::lookup(const std::string &filename) const {
    std::lock_guard<std::mutex> lock(p->dbMutex);
    return p->lookup(filename);
}

std::vector<MediaFile> MediaStore::query(const std::string &q, MediaType type, int limit) const {
    std::lock_guard<std::mutex> lock(p->dbMutex);
    return p->query(q, type, limit);
}

std::vector<Album> MediaStore::queryAlbums(const std::string &core_term, int limit) const {
    std::lock_guard<std::mutex> lock(p->dbMutex);
    return p->queryAlbums(core_term, limit);
}

std::vector<MediaFile> MediaStore::getAlbumSongs(const Album& album) const {
    std::lock_guard<std::mutex> lock(p->dbMutex);
    return p->getAlbumSongs(album);
}

std::string MediaStore::getETag(const std::string &filename) const {
    std::lock_guard<std::mutex> lock(p->dbMutex);
    return p->getETag(filename);
}

size_t MediaStore::size() const {
    std::lock_guard<std::mutex> lock(p->dbMutex);
    return p->size();
}

void MediaStore::pruneDeleted() {
    std::lock_guard<std::mutex> lock(p->dbMutex);
    p->pruneDeleted();
}

void MediaStore::archiveItems(const std::string &prefix) {
    std::lock_guard<std::mutex> lock(p->dbMutex);
    p->archiveItems(prefix);
}

void MediaStore::restoreItems(const std::string &prefix) {
    std::lock_guard<std::mutex> lock(p->dbMutex);
    p->restoreItems(prefix);
}

}
