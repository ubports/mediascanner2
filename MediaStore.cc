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

#include"MediaStore.hh"
#include"MediaFile.hh"
#include <sqlite3.h>
#include <stdio.h>

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

void create_tables(sqlite3 *db) {
    char *errmsg;
    sqlite3_exec(db, "CREATE VIRTUAL TABLE music USING fts4(filename, title, artist, album);",
            nullptr, nullptr, &errmsg);
    if(errmsg) {
        throw string(errmsg);
    }
}

int incrementer(void* arg, int /*num_cols*/, char **/*data*/, char **/*colnames*/) {
    (*((size_t*) arg))++;
    return 0;
}

MediaStore::MediaStore() {
    p = new MediaStorePrivate();
    // hackety hack
    string fname = "mediastore.db";
    ::remove(fname.c_str());
    if(sqlite3_open(fname.c_str(), &p->db) != SQLITE_OK) {
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
    if(sqlite3_exec(p->db, "SELECT * from music;", incrementer, &result, &err) != SQLITE_OK) {
        string s = err;
        throw s;
    }
    return result;
}

void MediaStore::insert(const MediaFile &m) {
    char *errmsg;
    p->files.push_back(m);
    // SQL injection here.
    const char *templ = "INSERT INTO music VALUES('%s', '%s', '%s', '%s');";
    char cmd[1024];
    string fname = m.getFileName();
    string title = m.getTitle();
    string author = m.getAuthor();
    string album = m.getAlbum();
    for(size_t i=0; i<fname.size(); i++) {
        if(fname[i] == '\'')
            fname[i] = ' ';
    }
    for(size_t i=0; i<title.size(); i++) {
        if(title[i] == '\'')
            title[i] = ' ';
    }
    for(size_t i=0; i<author.size(); i++) {
        if(author[i] == '\'')
            author[i] = ' ';
    }
    for(size_t i=0; i<album.size(); i++) {
        if(album[i] == '\'')
            album[i] = ' ';
    }
    sprintf(cmd, templ, fname.c_str(), title.c_str(), author.c_str(), album.c_str());
    if(sqlite3_exec(p->db, cmd, NULL, NULL, &errmsg) != SQLITE_OK) {
        string s = errmsg;
        throw s;
    }
    const char *typestr = m.getType() == AudioMedia ? "song" : "video";
    printf("Added %s to backing store: %s\n", typestr, m.getFileName().c_str());
    printf(" author : %s\n", m.getAuthor().c_str());
    printf(" title  : %s\n", m.getTitle().c_str());
    printf(" album  : %s\n", m.getAlbum().c_str());

}

void MediaStore::remove(const string &fname) {
    // Note: slow because fts does not do = very well.
    // Using MATCH may lead to inaccuracies.
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
    t->push_back(MediaFile(data[0], data[1], data[2], data[3]));
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
