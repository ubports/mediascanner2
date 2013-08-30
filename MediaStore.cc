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
    sqlite3_exec(db, "CREATE VIRTUAL TABLE music USING fts4(filename, title, artist);",
            nullptr, nullptr, &errmsg);
    if(errmsg) {
        throw string(errmsg);
    }
}

MediaStore::MediaStore() {
    p = new MediaStorePrivate();
    // hackety hack
    string fname = "mediastore.db";
    remove(fname.c_str());
    if(sqlite3_open(fname.c_str(), &p->db) != SQLITE_OK) {
        string s = sqlite3_errmsg(p->db);
        throw s;
    }
}

MediaStore::~MediaStore() {
    sqlite3_close(p->db);
    delete p;
}

void MediaStore::insert(MediaFile m) {
    p->files.push_back(m);
    const char *typestr = m.getType() == AudioMedia ? "song" : "video";
    printf("Added %s to backing store: %s\n", typestr, m.getFileName().c_str());
    printf(" author : %s\n", m.getAuthor().c_str());
    printf(" title  : %s\n", m.getTitle().c_str());
    printf(" album  : %s\n", m.getAlbum().c_str());
}

void MediaStore::remove(string m) {
    for(auto i=p->files.begin(); i!=p->files.end(); i++) {
        if((*i).getFileName() == m) {
            p->files.erase(i);
            printf("Removed from backing store: %s\n", m.c_str());
            return;
        }
    }
}


vector<MediaFile> MediaStore::query(const std::string &q) {
    vector<MediaFile> result;
    string lowerq = low(q);
    for(auto &c : p->files) {
        string l = low(c.getFileName());
        if(l.find(lowerq) != string::npos) {
            result.push_back(c);
        }
    }
    return result;
}
