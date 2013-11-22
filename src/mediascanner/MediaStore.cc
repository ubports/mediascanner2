/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * Authors:
 *    Jussi Pakkanen <jussi.pakkanen@canonical.com>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of version 3 of the GNU General Public License as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <stdexcept>

#include <glib.h>
#include <sqlite3.h>

#include "mozilla/fts3_tokenizer.h"
#include"MediaStore.hh"
#include"MediaFile.hh"
#include "Album.hh"
#include "sqliteutils.hh"
#include"utils.hh"

using namespace std;

// Increment this whenever changing db schema.
// It will cause dbstore to rebuild its tables.
static const int schemaVersion = 2;

struct MediaStorePrivate {
    sqlite3 *db;
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
    string deleteCmd(R"(DROP TABLE IF EXISTS media;
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
    title TEXT,
    date TEXT,
    artist TEXT,       -- Only relevant to audio
    album TEXT,        -- Only relevant to audio
    album_artist TEXT, -- Only relevant to audio
    track_number INTEGER, -- Only relevant to audio
    duration INTEGER,
    type INTEGER   -- 0=Audio, 1=Video
);

CREATE INDEX media_album_album_artist_idx ON media(album, album_artist);

CREATE TABLE media_attic (
    filename TEXT PRIMARY KEY NOT NULL,
    title TEXT,
    date TEXT,
    artist TEXT,    -- Only relevant to audio
    album TEXT,     -- Only relevant to audio
    album_artist TEXT, -- Only relevant to audio
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
        cachedir += "/mediascanner-test";
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

size_t MediaStore::size() const {
    Statement count(p->db, "SELECT COUNT(*) FROM media");
    count.step();
    return count.getInt(0);
}

void MediaStore::insert(const MediaFile &m) {
    Statement query(p->db, "INSERT OR REPLACE INTO media (filename, title, date, artist, album, album_artist, track_number, duration, type)  VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");
    string fname = m.getFileName();
    string title = m.getTitle();
    if(title.empty())
        title = filenameToTitle(fname);
    query.bind(1, fname);
    query.bind(2, title);
    query.bind(3, m.getDate());
    query.bind(4, m.getAuthor());
    query.bind(5, m.getAlbum());
    string album_artist = m.getAlbumArtist();
    if (album_artist.empty())
        album_artist = m.getAuthor();
    query.bind(6, album_artist);
    query.bind(7, m.getTrackNumber());
    query.bind(8, m.getDuration());
    query.bind(9, (int)m.getType());
    query.step();

    const char *typestr = m.getType() == AudioMedia ? "song" : "video";
    printf("Added %s to backing store: %s\n", typestr, m.getFileName().c_str());
    printf(" author   : '%s'\n", m.getAuthor().c_str());
    printf(" title    : %s\n", title.c_str());
    printf(" album    : '%s'\n", m.getAlbum().c_str());
    printf(" duration : %d\n", m.getDuration());
}

void MediaStore::remove(const string &fname) {
    Statement del(p->db, "DELETE FROM media WHERE filename = ?");
    del.bind(1, fname);
    del.step();
}

static vector<MediaFile> collect_media(Statement &query) {
    vector<MediaFile> result;
    while (query.step()) {
        const string filename = query.getText(0);
        const string title = query.getText(1);
        const string date = query.getText(2);
        const string author = query.getText(3);
        const string album = query.getText(4);
        const string album_artist = query.getText(5);
        int track_number = query.getInt(6);
        int duration = query.getInt(7);
        MediaType type = (MediaType)query.getInt(8);
        result.push_back(MediaFile(filename, title, date, author, album, album_artist, track_number, duration, type));
    }
    return result;
}

vector<MediaFile> MediaStore::query(const std::string &core_term, MediaType type) {
    Statement query(p->db, R"(
SELECT filename, title, date, artist, album, album_artist, track_number, duration, type
  FROM media JOIN (
    SELECT docid, rank(matchinfo(media_fts), 1.0, 0.5, 0.75) AS rank
      FROM media_fts WHERE media_fts MATCH ?
    ) AS ranktable ON (media.rowid = ranktable.docid)
  WHERE type == ?
  ORDER BY ranktable.rank DESC
)");
    query.bind(1, core_term + "*");
    query.bind(2, (int)type);
    return collect_media(query);
}

vector<Album> MediaStore::queryAlbums(const std::string &core_term) {
    Statement query(p->db, R"(
SELECT album, album_artist FROM media
WHERE rowid IN (SELECT docid FROM media_fts WHERE media_fts MATCH ?)
AND type == ? AND album <> ''
GROUP BY album, album_artist
)");
    query.bind(1, core_term + "*");
    query.bind(2, (int)AudioMedia);
    vector<Album> albums;
    while (query.step()) {
        const string album = query.getText(0);
        const string album_artist = query.getText(1);
        albums.push_back(Album(album, album_artist));
    }
    return albums;
}

vector<MediaFile> MediaStore::getAlbumSongs(const Album& album) {
    Statement query(p->db, R"(
SELECT filename, title, date, artist, album, album_artist, track_number, duration, type FROM media
WHERE album = ? AND album_artist = ? AND type = ?
ORDER BY track_number
)");
    query.bind(1, album.getTitle());
    query.bind(2, album.getArtist());
    query.bind(3, (int)AudioMedia);
    return collect_media(query);
}

void MediaStore::pruneDeleted() {
    vector<string> deleted;
    Statement query(p->db, "SELECT filename FROM media");
    while (query.step()) {
        const string filename = query.getText(0);
        FILE *f = fopen(filename.c_str(), "r");
        if(f) {
            fclose(f);
        } else {
            deleted.push_back(filename);
        }
    }
    query.finalize();
    printf("%d files deleted from disk.\n", (int)deleted.size());
    for(const auto &i : deleted) {
        remove(i);
    }
}

void MediaStore::archiveItems(const std::string &prefix) {
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
    if(sqlite3_exec(p->db, cmd, nullptr, nullptr, &errmsg) != SQLITE_OK) {
        sqlite3_exec(p->db, "ROLLBACK;", nullptr, nullptr, nullptr);
        throw runtime_error(errmsg);
    }
}

void MediaStore::restoreItems(const std::string &prefix) {
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
    if(sqlite3_exec(p->db, cmd, nullptr, nullptr, &errmsg) != SQLITE_OK) {
        sqlite3_exec(p->db, "ROLLBACK;", nullptr, nullptr, nullptr);
        throw runtime_error(errmsg);
    }

}
