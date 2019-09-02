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

#include "MediaStore.hh"

#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <mutex>
#include <sstream>
#include <map>

#include <glib.h>
#include <sqlite3.h>

#include "mozilla/fts3_tokenizer.h"
#include "MediaFile.hh"
#include "MediaFileBuilder.hh"
#include "Album.hh"
#include "Filter.hh"
#include "internal/sqliteutils.hh"
#include "internal/utils.hh"

using namespace std;

namespace mediascanner {

// Increment this whenever changing db schema.
// It will cause dbstore to rebuild its tables.
static const int schemaVersion = 10;

struct MediaStorePrivate {
    sqlite3 *db;
    // https://www.sqlite.org/cvstrac/wiki?p=DatabaseIsLocked
    // http://sqlite.com/faq.html#q6
    std::mutex dbMutex;

    void insert(const MediaFile &m) const;
    void remove(const std::string &fname) const;
    void insert_broken_file(const std::string &fname, const std::string &etag) const;
    void remove_broken_file(const std::string &fname) const;
    bool is_broken_file(const std::string &fname, const std::string &etag) const;
    MediaFile lookup(const std::string &filename) const;
    std::vector<MediaFile> query(const std::string &q, MediaType type, const Filter &filter) const;
    std::vector<Album> queryAlbums(const std::string &core_term, const Filter &filter) const;
    std::vector<string> queryArtists(const std::string &q, const Filter &filter) const;
    std::vector<MediaFile> getAlbumSongs(const Album& album) const;
    std::string getETag(const std::string &filename) const;
    std::vector<MediaFile> listSongs(const Filter &filter) const;
    std::vector<Album> listAlbums(const Filter &filter) const;
    std::vector<std::string> listArtists(const Filter &filter) const;
    std::vector<std::string> listAlbumArtists(const Filter &filter) const;
    std::vector<std::string> listGenres(const Filter &filter) const;
    bool hasMedia(MediaType type) const;

    size_t size() const;
    void pruneDeleted();
    void archiveItems(const std::string &prefix);
    void restoreItems(const std::string &prefix);
    void removeSubtree(const std::string &directory);

    void begin();
    void commit();
    void rollback();
};

extern "C" void sqlite3Fts3PorterTokenizerModule(
    sqlite3_tokenizer_module const**ppModule);

static void register_tokenizer(sqlite3 *db) {
    Statement query(db, "SELECT fts3_tokenizer(?, ?)");

    query.bind(1, "mozporter");
    const sqlite3_tokenizer_module *p = nullptr;
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

struct FirstContext {
    int type;
    union {
        int i;
        double f;
        struct {
            void *blob;
            int length;
        } b;
    } data;
};

static void first_step(sqlite3_context *ctx, int /*argc*/, sqlite3_value **argv) {
    FirstContext *d = static_cast<FirstContext*>(sqlite3_aggregate_context(ctx, sizeof(FirstContext*)));
    if (d->type != 0) {
        return;
    }
    sqlite3_value *arg = argv[0];
    d->type = sqlite3_value_type(arg);
    switch (d->type) {
    case SQLITE_INTEGER:
        d->data.i = sqlite3_value_int(arg);
        break;
    case SQLITE_FLOAT:
        d->data.f = sqlite3_value_double(arg);
        break;
    case SQLITE_NULL:
        break;
    case SQLITE_TEXT:
        d->data.b.length = sqlite3_value_bytes(arg);
        d->data.b.blob = malloc(d->data.b.length);
        memcpy(d->data.b.blob, sqlite3_value_text(arg), d->data.b.length);
        break;
    case SQLITE_BLOB:
        d->data.b.length = sqlite3_value_bytes(arg);
        d->data.b.blob = malloc(d->data.b.length);
        memcpy(d->data.b.blob, sqlite3_value_blob(arg), d->data.b.length);
        break;
    default:
        sqlite3_result_error(ctx, "Unhandled data type", -1);
        sqlite3_result_error_code(ctx, SQLITE_MISMATCH);
    }
}

static void first_finalize(sqlite3_context *ctx) {
    FirstContext *d = static_cast<FirstContext*>(sqlite3_aggregate_context(ctx, 0));
    if (d == nullptr) {
        sqlite3_result_null(ctx);
        return;
    }
    switch (d->type) {
    case SQLITE_INTEGER:
        sqlite3_result_int(ctx, d->data.i);
        break;
    case SQLITE_FLOAT:
        sqlite3_result_double(ctx, d->data.f);
        break;
    case SQLITE_NULL:
        sqlite3_result_null(ctx);
        break;
    case SQLITE_TEXT:
        sqlite3_result_text(ctx, reinterpret_cast<char*>(d->data.b.blob),
                            d->data.b.length, free);
        d->data.b.blob = nullptr;
        break;
    case SQLITE_BLOB:
        sqlite3_result_blob(ctx, d->data.b.blob, d->data.b.length, free);
        d->data.b.blob = nullptr;
        break;
    default:
        sqlite3_result_error(ctx, "Unhandled data type", -1);
        sqlite3_result_error_code(ctx, SQLITE_MISMATCH);
    }
}

static bool has_block_in_path(std::map<std::string, bool> &cache, const std::string &filename) {
    std::vector<std::string> path_segments;
    std::istringstream f(filename);
    std::string s;
    while (std::getline(f, s, '/')) {
        path_segments.push_back(s);
    }
    path_segments.pop_back();
    std::string trial_path;
    for(const auto &seg : path_segments) {
        if(trial_path != "/")
            trial_path += "/";
        trial_path += seg;
        auto r = cache.find(trial_path);
        if(r != cache.end() && r->second) {
            return true;
        }
        if(has_scanblock(trial_path)) {
            cache[trial_path] = true;
            return true;
        } else {
            cache[trial_path] = false;
        }
    }
    return false;
}

static void register_functions(sqlite3 *db) {
    if (sqlite3_create_function(db, "rank", -1, SQLITE_ANY, nullptr,
                                rankfunc, nullptr, nullptr) != SQLITE_OK) {
        throw runtime_error(sqlite3_errmsg(db));
    }

    if (sqlite3_create_function(db, "first", 1, SQLITE_ANY, nullptr,
                                nullptr, first_step, first_finalize) != SQLITE_OK) {
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
DROP TABLE IF EXISTS broken_files;
)");
    execute_sql(db, deleteCmd);
}

void createTables(sqlite3 *db) {
    string schema(R"(
CREATE TABLE schemaVersion (version INTEGER);

CREATE TABLE media (
    id INTEGER PRIMARY KEY,
    filename TEXT UNIQUE NOT NULL CHECK (filename LIKE '/%'),
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
    width INTEGER,        -- Only relevant to video/images
    height INTEGER,       -- Only relevant to video/images
    latitude DOUBLE,
    longitude DOUBLE,
    has_thumbnail INTEGER CHECK (has_thumbnail IN (0, 1)),
    mtime INTEGER,
    type INTEGER CHECK (type IN (1, 2, 3)) -- MediaType enum
);

CREATE INDEX media_type_idx ON media(type);
CREATE INDEX media_song_info_idx ON media(type, album_artist, album, disc_number, track_number, title) WHERE type = 1;
CREATE INDEX media_artist_idx ON media(type, artist) WHERE type = 1;
CREATE INDEX media_genre_idx ON media(type, genre) WHERE type = 1;
CREATE INDEX media_mtime_idx ON media(type, mtime);

CREATE TABLE media_attic (
    filename TEXT UNIQUE NOT NULL,
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
    width INTEGER,        -- Only relevant to video/images
    height INTEGER,       -- Only relevant to video/images
    latitude DOUBLE,
    longitude DOUBLE,
    has_thumbnail INTEGER,
    mtime INTEGER,
    type INTEGER   -- 0=Audio, 1=Video
);

CREATE VIRTUAL TABLE media_fts
USING fts4(content='media', title, artist, album, tokenize=mozporter);

CREATE TRIGGER media_bu BEFORE UPDATE ON media BEGIN
  DELETE FROM media_fts WHERE docid=old.id;
END;

CREATE TRIGGER media_au AFTER UPDATE ON media BEGIN
  INSERT INTO media_fts(docid, title, artist, album) VALUES (new.id, new.title, new.artist, new.album);
END;

CREATE TRIGGER media_bd BEFORE DELETE ON media BEGIN
  DELETE FROM media_fts WHERE docid=old.id;
END;

CREATE TRIGGER media_ai AFTER INSERT ON media BEGIN
  INSERT INTO media_fts(docid, title, artist, album) VALUES (new.id, new.title, new.artist, new.album);
END;

CREATE TABLE broken_files (
    filename TEXT PRIMARY KEY NOT NULL,
    etag TEXT NOT NULL
);
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
            std::string msg("Tried to open a db with schema version ");
            msg += std::to_string(detectedSchemaVersion);
            msg += ", while supported version is ";
            msg += std::to_string(schemaVersion) + ".";
            throw runtime_error(msg);
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
    Statement query(db, "INSERT OR REPLACE INTO media (filename, content_type, etag, title, date, artist, album, album_artist, genre, disc_number, track_number, duration, width, height, latitude, longitude, has_thumbnail, mtime, type)  VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
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
    query.bind(13, m.getWidth());
    query.bind(14, m.getHeight());
    query.bind(15, m.getLatitude());
    query.bind(16, m.getLongitude());
    query.bind(17, (int)m.getHasThumbnail());
    query.bind(18, (int64_t)m.getModificationTime());
    query.bind(19, (int)m.getType());
    query.step();

    const char *typestr = m.getType() == AudioMedia ? "song" : "video";
    printf("Added %s to backing store: %s\n", typestr, m.getFileName().c_str());
    printf(" author   : %s\n", m.getAuthor().c_str());
    printf(" title    : %s\n", m.getTitle().c_str());
    printf(" album    : %s\n", m.getAlbum().c_str());
    printf(" duration : %d\n", m.getDuration());

    // Not atomic with the addition above but very unlikely to crash between the two.
    // Even if it does, only one residual line remains and that will be cleaned up
    // on the next scan.
    remove_broken_file(m.getFileName());
}

void MediaStorePrivate::remove(const string &fname) const {
    Statement del(db, "DELETE FROM media WHERE filename = ?");
    del.bind(1, fname);
    del.step();
}

void MediaStorePrivate::insert_broken_file(const std::string &fname, const std::string &etag) const {
    Statement del(db, "INSERT OR REPLACE INTO broken_files (filename, etag) VALUES (?, ?)");
    del.bind(1, fname);
    del.bind(2, etag);
    del.step();
}

void MediaStorePrivate::remove_broken_file(const std::string &fname) const {
    Statement del(db, "DELETE FROM broken_files WHERE filename = ?");
    del.bind(1, fname);
    del.step();
}

bool MediaStorePrivate::is_broken_file(const std::string &fname, const std::string &etag) const {
    Statement query(db, "SELECT * FROM broken_files WHERE filename = ? AND etag = ?");
    query.bind(1, fname);
    query.bind(2, etag);
    return query.step();
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
        .setWidth(query.getInt(12))
        .setHeight(query.getInt(13))
        .setLatitude(query.getDouble(14))
        .setLongitude(query.getDouble(15))
        .setHasThumbnail(query.getInt(16))
        .setModificationTime(query.getInt64(17))
        .setType((MediaType)query.getInt(18));
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
SELECT filename, content_type, etag, title, date, artist, album, album_artist, genre, disc_number, track_number, duration, width, height, latitude, longitude, has_thumbnail, mtime, type
  FROM media
  WHERE filename = ?
)");
    query.bind(1, filename);
    if (!query.step()) {
        throw runtime_error("Could not find media " + filename);
    }
    return make_media(query);
}

vector<MediaFile> MediaStorePrivate::query(const std::string &core_term, MediaType type, const Filter &filter) const {
    string qs(R"(
SELECT filename, content_type, etag, title, date, artist, album, album_artist, genre, disc_number, track_number, duration, width, height, latitude, longitude, has_thumbnail, mtime, type
  FROM media
)");
    if (!core_term.empty()) {
        qs += R"(
  JOIN (
    SELECT docid, rank(matchinfo(media_fts), 1.0, 0.5, 0.75) AS rank
      FROM media_fts WHERE media_fts MATCH ?
    ) AS ranktable ON (media.id = ranktable.docid)
)";
    }
    qs += " WHERE type = ?";
    switch (filter.getOrder()) {
    case MediaOrder::Default:
    case MediaOrder::Rank:
        // We can only sort by rank if there was a query term
        if (!core_term.empty()) {
            qs += " ORDER BY ranktable.rank";
            if (!filter.getReverse()) { // Normal order is descending
                qs += " DESC";
            }
        }
        break;
    case MediaOrder::Title:
        qs += " ORDER BY title";
        if (filter.getReverse()) {
            qs += " DESC";
        }
        break;
    case MediaOrder::Date:
        qs += " ORDER BY date";
        if (filter.getReverse()) {
            qs += " DESC";
        }
        break;
    case MediaOrder::Modified:
        qs += " ORDER BY mtime";
        if (filter.getReverse()) {
            qs += " DESC";
        }
        break;
    }
    qs += " LIMIT ? OFFSET ?";

    Statement query(db, qs.c_str());
    int param = 1;
    if (!core_term.empty()) {
        query.bind(param++, core_term + "*");
    }
    query.bind(param++, (int)type);
    query.bind(param++, filter.getLimit());
    query.bind(param++, filter.getOffset());
    return collect_media(query);
}

static Album make_album(Statement &query) {
    const string album = query.getText(0);
    const string album_artist = query.getText(1);
    const string date = query.getText(2);
    const string genre = query.getText(3);
    const string filename = query.getText(4);
    const bool has_thumbnail = query.getInt(5);
    const int artist_count = query.getInt(6);
    return Album(album, album_artist, date, genre, filename, has_thumbnail, artist_count);
}

static vector<Album> collect_albums(Statement &query) {
    vector<Album> result;
    while (query.step()) {
        result.push_back(make_album(query));
    }
    return result;
}

vector<Album> MediaStorePrivate::queryAlbums(const std::string &core_term, const Filter &filter) const {
    string qs(R"(
SELECT album, album_artist, first(date) as date, first(genre) as genre, first(filename) as filename, first(has_thumbnail) as has_thumbnail, count(distinct album_artist) as artist_count, first(mtime) as mtime FROM media
WHERE type = ? AND album <> ''
)");
    if (!core_term.empty()) {
        qs += " AND id IN (SELECT docid FROM media_fts WHERE media_fts MATCH ?)";
    }
    qs += " GROUP BY album";
    switch (filter.getOrder()) {
    case MediaOrder::Default:
    case MediaOrder::Title:
        qs += " ORDER BY album";
        if (filter.getReverse()) {
            qs += " DESC";
        }
        break;
    case MediaOrder::Rank:
        throw std::runtime_error("Can not query albums by rank");
    case MediaOrder::Date:
        throw std::runtime_error("Can not query albums by date");
    case MediaOrder::Modified:
        qs += " ORDER BY mtime";
        if (filter.getReverse()) {
            qs += " DESC";
        }
        break;
    }
    qs += " LIMIT ? OFFSET ?";

    Statement query(db, qs.c_str());
    int param = 1;
    query.bind(param++, (int)AudioMedia);
    if (!core_term.empty()) {
        query.bind(param++, core_term + "*");
    }
    query.bind(param++, filter.getLimit());
    query.bind(param++, filter.getOffset());
    return collect_albums(query);
}

vector<string> MediaStorePrivate::queryArtists(const string &q, const Filter &filter) const {
    string qs(R"(
SELECT artist FROM media
WHERE type = ? AND artist <> ''
)");
    if (!q.empty()) {
        qs += "AND id IN (SELECT docid FROM media_fts WHERE media_fts MATCH ?)";
    }
    qs += " GROUP BY artist";
    switch (filter.getOrder()) {
    case MediaOrder::Default:
    case MediaOrder::Title:
        qs += " ORDER BY artist";
        if (filter.getReverse()) {
            qs += " DESC";
        }
        break;
    case MediaOrder::Rank:
        throw std::runtime_error("Can not query artists by rank");
    case MediaOrder::Date:
        throw std::runtime_error("Can not query artists by date");
    case MediaOrder::Modified:
        throw std::runtime_error("Can not query artists by modification date");
    }
    qs += " LIMIT ? OFFSET ?";

    Statement query(db, qs.c_str());
    int param = 1;
    query.bind(param++, (int)AudioMedia);
    if (!q.empty()) {
        query.bind(param++, q + "*");
    }
    query.bind(param++, filter.getLimit());
    query.bind(param++, filter.getOffset());
    vector<string> result;
    while (query.step()) {
        result.push_back(query.getText(0));
    }
    return result;
}

vector<MediaFile> MediaStorePrivate::getAlbumSongs(const Album& album) const {
    Statement query(db, R"(
SELECT filename, content_type, etag, title, date, artist, album, album_artist, genre, disc_number, track_number, duration, width, height, latitude, longitude, has_thumbnail, mtime, type FROM media
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

std::vector<MediaFile> MediaStorePrivate::listSongs(const Filter &filter) const {
    std::string qs(R"(
SELECT filename, content_type, etag, title, date, artist, album, album_artist, genre, disc_number, track_number, duration, width, height, latitude, longitude, has_thumbnail, mtime, type
  FROM media
  WHERE type = ?
)");
    if (filter.hasArtist()) {
        qs += " AND artist = ?";
    }
    if (filter.hasAlbum()) {
        qs += " AND album = ?";
    }
    if (filter.hasAlbumArtist()) {
        qs += " AND album_artist = ?";
    }
    if (filter.hasGenre()) {
        qs += " AND genre = ?";
    }
    qs += R"(
ORDER BY album_artist, album, disc_number, track_number, title
LIMIT ? OFFSET ?
)";
    Statement query(db, qs.c_str());
    int param = 1;
    query.bind(param++, (int)AudioMedia);
    if (filter.hasArtist()) {
        query.bind(param++, filter.getArtist());
    }
    if (filter.hasAlbum()) {
        query.bind(param++, filter.getAlbum());
    }
    if (filter.hasAlbumArtist()) {
        query.bind(param++, filter.getAlbumArtist());
    }
    if (filter.hasGenre()) {
        query.bind(param++, filter.getGenre());
    }
    query.bind(param++, filter.getLimit());
    query.bind(param++, filter.getOffset());

    return collect_media(query);
}

std::vector<Album> MediaStorePrivate::listAlbums(const Filter &filter) const {
    std::string qs(R"(
SELECT album, album_artist, first(date) as date, first(genre) as genre, first(filename) as filename, first(has_thumbnail) as has_thumbnail, count(distinct album_artist) as artist_count FROM media
  WHERE type = ?
)");
    if (filter.hasArtist()) {
        qs += " AND artist = ?";
    }
    if (filter.hasAlbumArtist()) {
        qs += " AND album_artist = ?";
    }
    if (filter.hasGenre()) {
        qs += "AND genre = ?";
    }
    qs += R"(
GROUP BY album
ORDER BY album
LIMIT ? OFFSET ?
)";
    Statement query(db, qs.c_str());
    int param = 1;
    query.bind(param++, (int)AudioMedia);
    if (filter.hasArtist()) {
        query.bind(param++, filter.getArtist());
    }
    if (filter.hasAlbumArtist()) {
        query.bind(param++, filter.getAlbumArtist());
    }
    if (filter.hasGenre()) {
        query.bind(param++, filter.getGenre());
    }
    query.bind(param++, filter.getLimit());
    query.bind(param++, filter.getOffset());

    return collect_albums(query);
}

vector<std::string> MediaStorePrivate::listArtists(const Filter &filter) const {
    string qs(R"(
SELECT artist FROM media
  WHERE type = ?
)");
    if (filter.hasGenre()) {
        qs += " AND genre = ?";
    }
    qs += R"(
  GROUP BY artist
  ORDER BY artist
  LIMIT ? OFFSET ?
)";
    Statement query(db, qs.c_str());
    int param = 1;
    query.bind(param++, (int)AudioMedia);
    if (filter.hasGenre()) {
        query.bind(param++, filter.getGenre());
    }
    query.bind(param++, filter.getLimit());
    query.bind(param++, filter.getOffset());

    vector<string> artists;
    while (query.step()) {
        artists.push_back(query.getText(0));
    }
    return artists;
}

vector<std::string> MediaStorePrivate::listAlbumArtists(const Filter &filter) const {
    string qs(R"(
SELECT album_artist FROM media
  WHERE type = ?
)");
    if (filter.hasGenre()) {
        qs += " AND genre = ?";
    }
    qs += R"(
  GROUP BY album_artist
  ORDER BY album_artist
  LIMIT ? OFFSET ?
)";
    Statement query(db, qs.c_str());
    int param = 1;
    query.bind(param++, (int)AudioMedia);
    if (filter.hasGenre()) {
        query.bind(param++, filter.getGenre());
    }
    query.bind(param++, filter.getLimit());
    query.bind(param++, filter.getOffset());

    vector<string> artists;
    while (query.step()) {
        artists.push_back(query.getText(0));
    }
    return artists;
}

vector<std::string> MediaStorePrivate::listGenres(const Filter &filter) const {
    Statement query(db, R"(
SELECT genre FROM media
  WHERE type = ?
  GROUP BY genre
  ORDER BY genre
  LIMIT ? OFFSET ?
)");
    query.bind(1, (int)AudioMedia);
    query.bind(2, filter.getLimit());
    query.bind(3, filter.getOffset());

    vector<string> genres;
    while (query.step()) {
        genres.push_back(query.getText(0));
    }
    return genres;
}

bool MediaStorePrivate::hasMedia(MediaType type) const {
    if (type == AllMedia) {
        Statement query(db, R"(
SELECT id FROM media
  LIMIT 1
)");
        return query.step();
    } else {
        Statement query(db, R"(
SELECT id FROM media
  WHERE type = ?
  LIMIT 1
)");
        query.bind(1, (int)type);
        return query.step();
    }
}

void MediaStorePrivate::pruneDeleted() {
    std::map<std::string, bool> path_cache;
    vector<string> deleted;
    Statement query(db, "SELECT filename FROM media");
    while (query.step()) {
        const string filename = query.getText(0);
        if (access(filename.c_str(), F_OK) != 0 ||
            has_block_in_path(path_cache, filename)) {
            deleted.push_back(filename);
            continue;
        }
    }
    query.finalize();
    printf("%d files deleted from disk or in scanblocked directories.\n", (int)deleted.size());
    for(const auto &i : deleted) {
        remove(i);
    }
}

void MediaStorePrivate::archiveItems(const std::string &prefix) {
    const char *templ = R"(BEGIN TRANSACTION;
INSERT INTO media_attic (filename, content_type, etag, title, date, artist, album, album_artist, genre, disc_number, track_number, duration, width, height, latitude, longitude, has_thumbnail, mtime, type)
  SELECT filename, content_type, etag, title, date, artist, album, album_artist, genre, disc_number, track_number, duration, width, height, latitude, longitude, has_thumbnail, mtime, type
    FROM media WHERE filename LIKE %s;
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
INSERT INTO media (filename, content_type, etag, title, date, artist, album, album_artist, genre, disc_number, track_number, duration, width, height, latitude, longitude, has_thumbnail, mtime, type)
  SELECT filename, content_type, etag, title, date, artist, album, album_artist, genre, disc_number, track_number, duration, width, height, latitude, longitude, has_thumbnail, mtime, type
    FROM media_attic WHERE filename LIKE %s;
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

void MediaStorePrivate::removeSubtree(const std::string &directory) {
    string escaped = directory;
    string::size_type pos = 0;
    // Escape instances of like expression special characters and the escape character.
    while (true) {
        pos = escaped.find_first_of("%_!", pos);
        if (pos == string::npos) {
            break;
        }
        escaped.replace(pos, 0, "!");
        pos += 2;
    }
    if (escaped.empty() || escaped[escaped.size() - 1] != '/') {
        escaped += '/';
    }
    escaped += '%';

    Statement query(db, "DELETE FROM media WHERE filename LIKE ? ESCAPE '!'");
    query.bind(1, escaped);
    query.step();
}

void MediaStorePrivate::begin() {
    Statement query(db, "BEGIN TRANSACTION");
    query.step();
}

void MediaStorePrivate::commit() {
    Statement query(db, "COMMIT TRANSACTION");
    query.step();
}

void MediaStorePrivate::rollback() {
    Statement query(db, "ROLLBACK TRANSACTION");
    query.step();
}

void MediaStore::insert(const MediaFile &m) const {
    std::lock_guard<std::mutex> lock(p->dbMutex);
    p->insert(m);
}

void MediaStore::remove(const std::string &fname) const {
    std::lock_guard<std::mutex> lock(p->dbMutex);
    p->remove(fname);
}

void MediaStore::insert_broken_file(const std::string &fname, const std::string &etag) const {
    std::lock_guard<std::mutex> lock(p->dbMutex);
    p->insert_broken_file(fname, etag);
}

void MediaStore::remove_broken_file(const std::string &fname) const {
    std::lock_guard<std::mutex> lock(p->dbMutex);
    p->remove_broken_file(fname);
}

bool MediaStore::is_broken_file(const std::string &fname, const std::string &etag) const {
    std::lock_guard<std::mutex> lock(p->dbMutex);
    return p->is_broken_file(fname, etag);
}

MediaFile MediaStore::lookup(const std::string &filename) const {
    std::lock_guard<std::mutex> lock(p->dbMutex);
    return p->lookup(filename);
}

std::vector<MediaFile> MediaStore::query(const std::string &q, MediaType type, const Filter &filter) const {
    std::lock_guard<std::mutex> lock(p->dbMutex);
    return p->query(q, type, filter);
}

std::vector<Album> MediaStore::queryAlbums(const std::string &core_term, const Filter &filter) const {
    std::lock_guard<std::mutex> lock(p->dbMutex);
    return p->queryAlbums(core_term, filter);
}

std::vector<string> MediaStore::queryArtists(const std::string &q, const Filter &filter) const {
    std::lock_guard<std::mutex> lock(p->dbMutex);
    return p->queryArtists(q, filter);
}

std::vector<MediaFile> MediaStore::getAlbumSongs(const Album& album) const {
    std::lock_guard<std::mutex> lock(p->dbMutex);
    return p->getAlbumSongs(album);
}

std::string MediaStore::getETag(const std::string &filename) const {
    std::lock_guard<std::mutex> lock(p->dbMutex);
    return p->getETag(filename);
}

std::vector<MediaFile> MediaStore::listSongs(const Filter &filter) const {
    std::lock_guard<std::mutex> lock(p->dbMutex);
    return p->listSongs(filter);
}

std::vector<Album> MediaStore::listAlbums(const Filter &filter) const {
    std::lock_guard<std::mutex> lock(p->dbMutex);
    return p->listAlbums(filter);
}

std::vector<std::string> MediaStore::listArtists(const Filter &filter) const {
    std::lock_guard<std::mutex> lock(p->dbMutex);
    return p->listArtists(filter);
}

std::vector<std::string> MediaStore::listAlbumArtists(const Filter &filter) const {
    std::lock_guard<std::mutex> lock(p->dbMutex);
    return p->listAlbumArtists(filter);
}

std::vector<std::string> MediaStore::listGenres(const Filter &filter) const {
    std::lock_guard<std::mutex> lock(p->dbMutex);
    return p->listGenres(filter);
}

bool MediaStore::hasMedia(MediaType type) const {
    std::lock_guard<std::mutex> lock(p->dbMutex);
    return p->hasMedia(type);
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

void MediaStore::removeSubtree(const std::string &directory) {
    std::lock_guard<std::mutex> lock(p->dbMutex);
    p->removeSubtree(directory);
}

MediaStoreTransaction MediaStore::beginTransaction() {
    std::lock_guard<std::mutex> lock(p->dbMutex);
    p->begin();
    return MediaStoreTransaction(p);
}

MediaStoreTransaction::MediaStoreTransaction(MediaStorePrivate *p)
    : p(p) {
}

MediaStoreTransaction::MediaStoreTransaction(MediaStoreTransaction &&other) {
    *this = std::move(other);
}

MediaStoreTransaction::~MediaStoreTransaction() {
    if (!p) {
        return;
    }
    std::lock_guard<std::mutex> lock(p->dbMutex);
    try {
        p->rollback();
    } catch (const std::exception &e) {
        fprintf(stderr, "MediaStoreTransaction: error rolling back in destructor: %s\n", e.what());
    }
}

MediaStoreTransaction& MediaStoreTransaction::operator=(MediaStoreTransaction &&other) {
    p = other.p;
    other.p = nullptr;
    return *this;
}

void MediaStoreTransaction::commit() {
    std::lock_guard<std::mutex> lock(p->dbMutex);
    p->commit();
    p->begin();
}

}
