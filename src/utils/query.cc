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

#include "mediascanner/Filter.hh"
#include "mediascanner/MediaFile.hh"
#include "mediascanner/MediaStore.hh"

#include<stdio.h>
#include<string>
#include<memory>
#include<vector>

using namespace std;
using namespace mediascanner;

void queryDb(const string &core_term) {
    MediaStore store(MS_READ_ONLY);
    vector<MediaFile> results;
    results = store.query(core_term, AudioMedia, Filter());
    if(results.empty()) {
        printf("No audio matches.\n");
    } else {
        printf("Audio matches:\n");
    }
    for(const auto &i : results) {
        printf("Filename: %s\n", i.getFileName().c_str());
    }

    results = store.query(core_term, VideoMedia, Filter());
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
    string core_term(argv[1]);
    queryDb(core_term);
}

