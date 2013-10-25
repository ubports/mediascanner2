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

#include "mozilla/fts3_tokenizer.h"
#include"MediaStore.hh"
#include"MediaFile.hh"
#include"utils.hh"
#include <sqlite3.h>
#include <cstdio>

using namespace std;

static string low(const string &s) {
    string ls = s;
    for(size_t i=0; i<ls.size(); i++) {
        ls[i] = tolower(ls[i]);
    }
    return ls;
}

struct MediaStorePrivate {
    vector<MediaFile> files;
    sqlite3 *db;
};

extern "C" void sqlite3Fts3PorterTokenizerModule(
    sqlite3_tokenizer_module const**ppModule);

int register_tokenizer(sqlite3 *db) {
    int rc;
    const sqlite3_tokenizer_module *p = NULL;
    sqlite3_stmt *pStmt;
    const char *zSql = "SELECT fts3_tokenizer(?, ?)";

    rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
    if( rc!=SQLITE_OK ){
        return rc;
    }

    sqlite3_bind_text(pStmt, 1, "mozporter", -1, SQLITE_STATIC);
    sqlite3Fts3PorterTokenizerModule(&p);
    sqlite3_bind_blob(pStmt, 2, &p, sizeof(p), SQLITE_STATIC);
    sqlite3_step(pStmt);

    return sqlite3_finalize(pStmt);
}

static void execute_sql(sqlite3 *db, const string &cmd) {
    char *errmsg;
    sqlite3_exec(db, cmd.c_str(), nullptr, nullptr, &errmsg);
    if(errmsg) {
        throw string(errmsg);
    }
}

void create_tables(sqlite3 *db) {
    string musicCreate("CREATE TABLE IF NOT EXISTS music (filename TEXT PRIMARY KEY, title TEXT, artist TEXT, album TEXT, duration INT);");
    string musicFtsCreate("CREATE VIRTUAL TABLE IF NOT EXISTS music_fts USING fts4(content='music', title, artist, album, tokenize=mozporter);");
    string videoCreate("CREATE TABLE IF NOT EXISTS video (filename TEXT PRIMARY KEY, title TEXT, duration INT);");
    string videoFtsCreate("CREATE VIRTUAL TABLE IF NOT EXISTS video_fts USING fts4(content='video', title, tokenize=mozporter);");

    string mTC("CREATE TRIGGER IF NOT EXISTS music_bu BEFORE UPDATE ON music BEGIN\n");
    mTC += "  DELETE FROM music_fts WHERE docid=old.rowid;\nEND;\n";
    mTC += "CREATE TRIGGER IF NOT EXISTS music_bd BEFORE DELETE ON music BEGIN\n";
    mTC += "  DELETE FROM music_fts WHERE docid=old.rowid;\nEND;\n";
    mTC += "CREATE TRIGGER IF NOT EXISTS music_au AFTER UPDATE ON music BEGIN\n";
    mTC += "  INSERT INTO music_fts(docid, title, artist, album) VALUES(new.rowid, new.title, new.artist, new.album);\n";
    mTC += "END;\n";
    mTC += "CREATE TRIGGER music_ai AFTER INSERT ON music BEGIN\n";
    mTC += "  INSERT INTO music_fts(docid, title, artist, album) VALUES(new.rowid, new.title, new.artist, new.album);\n";
    mTC += "END;";

    string vTC("CREATE TRIGGER IF NOT EXISTS video_bu BEFORE UPDATE ON video BEGIN\n");
    vTC += "  DELETE FROM video_fts WHERE docid=old.rowid;\nEND;\n";
    vTC += "CREATE TRIGGER IF NOT EXISTS video_bd BEFORE DELETE ON video BEGIN\n";
    vTC += "  DELETE FROM video_fts WHERE docid=old.rowid;\nEND;\n";
    vTC += "CREATE TRIGGER IF NOT EXISTS video_au AFTER UPDATE ON video BEGIN\n";
    vTC += "  INSERT INTO video_fts(docid, title) VALUES(new.rowid, new.title);\n";
    vTC += "END;\n";
    vTC += "CREATE TRIGGER video_ai AFTER INSERT ON video BEGIN\n";
    vTC += "  INSERT INTO video_fts(docid, title) VALUES(new.rowid, new.title);\n";
    vTC += "END;";
//    printf("%s", tC.c_str());
    execute_sql(db, musicCreate);
    execute_sql(db, musicFtsCreate);
    execute_sql(db, videoCreate);
    execute_sql(db, videoFtsCreate);
    execute_sql(db, mTC);
    execute_sql(db, vTC);
}

int incrementer(void* arg, int /*num_cols*/, char **/*data*/, char **/*colnames*/) {
    (*((size_t*) arg))++;
    return 0;
}

MediaStore::MediaStore(const std::string &filename_base) {
    p = new MediaStorePrivate();
    string fname = filename_base + "-mediastore.db";
    if(sqlite3_open(fname.c_str(), &p->db) != SQLITE_OK) {
        string s = sqlite3_errmsg(p->db);
        throw s;
    }
    if (register_tokenizer(p->db) != SQLITE_OK) {
        string s = sqlite3_errmsg(p->db);
        throw s;
    }
    create_tables(p->db);
}

MediaStore::~MediaStore() {
    sqlite3_close(p->db);
    delete p;
}

size_t MediaStore::size() const {
    size_t result = 0;
    char *err;
    if(sqlite3_exec(p->db, "SELECT * FROM music;", incrementer, &result, &err) != SQLITE_OK) {
        string s = err;
        throw s;
    }
    if(sqlite3_exec(p->db, "SELECT * FROM video;", incrementer, &result, &err) != SQLITE_OK) {
        string s = err;
        throw s;
    }
    return result;
}

static int yup(void* arg, int /*num_cols*/, char **/*data*/, char ** /*colnames*/) {
    bool *t = reinterpret_cast<bool *> (arg);
    *t = true;
    return 0;
}

void MediaStore::insert(const MediaFile &m) {
    char *errmsg;
    p->files.push_back(m);
    // SQL injection here.
    const char *musicinsert_templ = "INSERT INTO music VALUES(%s, %s, %s, %s, %s);";
    const char *videoinsert_templ = "INSERT INTO video VALUES(%s, %s, %s);";
    const char *query_templ = "SELECT * FROM %s WHERE filename=%s;";
    const size_t bufsize = 1024;
    char qcmd[bufsize];
    char icmd[bufsize];

    string fname = sqlQuote(m.getFileName());
    string title;
    if(m.getTitle().empty())
        title = sqlQuote(filenameToTitle(m.getFileName()));
    else
        title = sqlQuote(m.getTitle());
    string author = sqlQuote(m.getAuthor());
    string album = sqlQuote(m.getAlbum());
    string duration = to_string(m.getDuration());

    if(m.getType() == AudioMedia) {
        snprintf(qcmd, bufsize, query_templ, "music", fname.c_str());
        snprintf(icmd, bufsize, musicinsert_templ, fname.c_str(), title.c_str(),
                author.c_str(), album.c_str(), duration.c_str());
    } else if(m.getType() == VideoMedia) {
        snprintf(qcmd, bufsize, query_templ, "video", fname.c_str());
        snprintf(icmd, bufsize, videoinsert_templ, fname.c_str(), title.c_str(), duration.c_str());
    } else {
        return;
    }
    bool was_in = false;
    if(sqlite3_exec(p->db, qcmd, yup, &was_in, &errmsg ) != SQLITE_OK) {
        string s = errmsg;
        throw s;
    }
    if(was_in) {
        return;
    }
    if(sqlite3_exec(p->db, icmd, NULL, NULL, &errmsg) != SQLITE_OK) {
        string s = errmsg;
        throw s;
    }
    const char *typestr = m.getType() == AudioMedia ? "song" : "video";
    printf("Added %s to backing store: %s\n", typestr, m.getFileName().c_str());
    printf(" author   : '%s'\n", m.getAuthor().c_str());
    printf(" title    : %s\n", title.c_str());
    printf(" album    : '%s'\n", m.getAlbum().c_str());
    printf(" duration : %d\n", m.getDuration());
}

void MediaStore::remove(const string &fname) {
    const char *templ = "DELETE FROM music WHERE filename = '%s';";
    char cmd[1024];
    sprintf(cmd, templ, fname.c_str());
    char *errmsg;
    if(sqlite3_exec(p->db, cmd, NULL, NULL, &errmsg) != SQLITE_OK) {
        string s = errmsg;
        throw s;
    }
}

static int music_adder(void* arg, int /*num_cols*/, char **data, char ** /*colnames*/) {
    vector<MediaFile> *t = reinterpret_cast<vector<MediaFile> *> (arg);
    int duration = 0; // TMP
    t->push_back(MediaFile(data[0], data[1], data[2], data[3], duration, AudioMedia));
    return 0;
}

vector<MediaFile> MediaStore::query(const std::string &q) {
    vector<MediaFile> result;
    const char *templ = "SELECT * FROM music WHERE title MATCH '%s*';";
    char cmd[1024];
    sprintf(cmd, templ, q.c_str());
    char *errmsg;
    if(sqlite3_exec(p->db, cmd, music_adder, &result, &errmsg) != SQLITE_OK) {
        string s = errmsg;
        throw s;
    }
    return result;
}

static int deleteChecker(void* arg, int /*num_cols*/, char **data, char ** /*colnames*/) {
    vector<string> *t = reinterpret_cast<vector<string>*> (arg);
    const char *fname = data[0];
    FILE *f = fopen(fname, "r");
    if(f) {
        fclose(f);
    } else {
        t->push_back(fname);
    }
    return 0;
}

void MediaStore::pruneDeleted() {
    vector<string> deleted;
    const char *query = "SELECT filename FROM music;";
    char *errmsg;
    if(sqlite3_exec(p->db, query, deleteChecker, &deleted, &errmsg) != SQLITE_OK) {
        string s = errmsg;
        throw s;
    }
    printf("%d files deleted from disk.\n", (int)deleted.size());
    for(const auto &i : deleted) {
        remove(i);
    }
}

