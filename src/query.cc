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
#include"MediaFile.hh"
#include"MediaStore.hh"

#include<sqlite3.h>
#include<stdio.h>
#include<string>
#include<memory>
#include<sys/stat.h>
#include<dirent.h>
#include<unistd.h>
#include<vector>

using namespace std;

void queryDb(const string &file_base, const string &core_term) {
    MediaStore store(file_base);
    vector<MediaFile> results;
    results = store.query(core_term, AudioMedia);
    if(results.empty()) {
        printf("No audio matches.\n");
    } else {
        printf("Audio matches:\n");
    }
    for(const auto &i : results) {
        printf("Filename: %s\n", i.getFileName().c_str());
    }

    results = store.query(core_term, VideoMedia);
    if(results.empty()) {
        printf("No video matches.\n");
    } else {
        printf("Video matches:\n");
    }
    for(const auto &i : results) {
        printf("Filename: %s\n", i.getFileName().c_str());
    }
}

int main(int argc, char **argv) {
    if(argc < 2) {
        printf("%s <term>\n", argv[0]);
        return 1;
    }
    string audioname = "home-music";
    string videoname = "home-video";
    string term = argv[1];
    printf("Results from music directory\n");
    queryDb(audioname, term);
    printf("Results from video directory\n");
    queryDb(videoname, term);
    char cwd[1024];
    getcwd(cwd, 1024);
    // FIXME in this test app use current dir.
    string mountDir(cwd);
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
            string dbfile = dbCandidate + "-mediastore.db";
            if(stat(dbfile.c_str(), &statbuf) == 0) {
                printf("Results from external volume %s\n", fname.c_str());
                queryDb(dbCandidate, term);
            }
        }
    }
}
