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

#include"utils.hh"

#include<sqlite3.h>
#include<stdio.h>
#include<string>
#include<memory>
#include<sys/stat.h>
#include<dirent.h>

using namespace std;

int printer(void*/*arg*/, int num_cols, char **data, char **colnames) {
    for(int i=0; i<num_cols; i++) {
        printf("%s: %s\n", colnames[i], data[i]);
    }
    return 0;
}

int queryDb(const string &fname, const string &core_term) {
    sqlite3 *db;
    const char *music_templ = "SELECT * FROM music WHERE artist MATCH %s UNION SELECT * FROM music WHERE title MATCH %s;";
    const char *video_templ = "SELECT * FROM video WHERE title MATCH %s;";
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
    printf("\nVideo results\n\n");
    sprintf(cmd, video_templ, term.c_str());
    if(sqlite3_exec(db, cmd, printer, NULL, &err) != SQLITE_OK) {
        printf("%s\n", sqlite3_errmsg(db));
        return 1;
    }
    sqlite3_close(db);
    return 0;
}

int main(int argc, char **argv) {
    if(argc < 2) {
        printf("%s <term>\n", argv[0]);
        return 1;
    }
    string audioname = "home-music-mediastore.db";
    string videoname = "home-video-mediastore.db";
    string term = argv[1];
    printf("Results from music directory\n");
    queryDb(audioname, term);
    printf("Results from video directory\n");
    queryDb(videoname, term);
    string mountDir("/home/jpakkane/workspace/scantest/build");
    unique_ptr<DIR, int(*)(DIR*)> dir(opendir(mountDir.c_str()), closedir);
    unique_ptr<struct dirent, void(*)(void*)> entry((struct dirent*)malloc(sizeof(struct dirent) + NAME_MAX),
                free);
    struct dirent *de;
    while(readdir_r(dir.get(), entry.get(), &de) == 0 && de ) {
        struct stat statbuf;
        string fname = entry.get()->d_name;
        if(fname[0] == '.')
            continue;
        string fullpath = mountDir + "/" + fname;
        stat(fullpath.c_str(), &statbuf);
        if(S_ISDIR(statbuf.st_mode)) {
            string dbCandidate("mount-");
            dbCandidate += fname;
            dbCandidate += "-mediastore.db";
            if(stat(dbCandidate.c_str(), &statbuf) == 0) {
                printf("Results from external volume %s\n", fname.c_str());
                queryDb(dbCandidate, term);
            }
        }
    }
}
