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
#include "utils.hh"

#include<vector>
#include<string>
#include<fstream>
#include<cstdio>
#include<random>
#include<sqlite3.h>

using namespace std;

#define RNDWORD words[rnd() % words.size()]

int printer(void*/*arg*/, int num_cols, char **data, char **colnames) {
    for(int i=0; i<num_cols; i++) {
        printf("%s: %s\n", colnames[i], data[i]);
    }
    return 0;
}

int queryDb(const string &fname, const string &core_term) {
    sqlite3 *db;
    const char *music_templ = "SELECT * FROM music WHERE rowid IN (SELECT docid FROM music_fts WHERE artist MATCH %s UNION SELECT docid FROM music_fts WHERE title MATCH %s);";
    string term = sqlQuote(core_term + "*");
    if(sqlite3_open_v2(fname.c_str(), &db, SQLITE_OPEN_READONLY, nullptr) != SQLITE_OK) {
        printf("%s\n", sqlite3_errmsg(db));
        return 1;
    }
    char cmd[1024];
    sprintf(cmd, music_templ, term.c_str(), term.c_str());
    char *err;
    printf("Music results\n\n");
    if(sqlite3_exec(db, cmd, printer, NULL, &err) != SQLITE_OK) {
        printf("%s\n", sqlite3_errmsg(db));
        return 1;
    }
    return 0;
}

int createDb(const string &base) {
    MediaStore store(base, MS_READ_WRITE);
    ifstream ifile("/usr/share/dict/words");
    string line;
    vector<string> words;
    random_device rnd;
    while(getline(ifile, line)) {
        if(line.length() >=3) {
            if(line[line.length()-2] == '\'')
                continue;
        }
        words.push_back(line);
    }
    int i=0;
    for(int artistCount=0; artistCount < 1000; artistCount++) {
        string artist = RNDWORD + " " + RNDWORD;
        for(int albumCount=0; albumCount < 3; albumCount++) {
            string album = RNDWORD + " " + RNDWORD + " " + RNDWORD;
            for(int trackCount=0; trackCount < 10; trackCount++) {
                int numWords = rnd() % 4 + 1;
                string track(RNDWORD);
                for(int i=1; i<numWords; i++) {
                    track += " " + RNDWORD;
                }
                string fname = to_string(i) + ".mp3";
                MediaFile mf(fname, track, artist, album, rnd() % 300, AudioMedia);
                store.insert(mf);
                i++;
                //printf("%s, %s, %s\n", artist.c_str(), album.c_str(), track.c_str());
            }
        }
    }
    return 0;
}

int main(int argc, char **argv) {
    if(argc == 1) {
        return createDb("scaletest");
    } else if(argc == 2) {
        return queryDb("scaletest-mediastore.db", argv[1]);
    } else {
        printf("Read the source to understand.\n");
        return 1;
    }
}
