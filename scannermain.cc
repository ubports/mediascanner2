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

#include "Scanner.hh"
#include "MediaFile.hh"
#include "MediaStore.hh"

#include <cstdlib>
#include <cstdio>

using namespace std;

static int do_query(vector<string> &files, string &query) {
    MediaStore s;
    for(auto &fname : files) {
        s.insert(MediaFile(fname));
    }
    vector<MediaFile> matches = s.query(query);
    printf("Got %ld matches.\n", (long)matches.size());
    for(auto &m : matches) {
        printf(" %s\n", m.getFileName().c_str());
    }
    return 0;
}

int main(int argc, char **argv) {
    if(argc != 2) {
        printf("%s <query term>\n", argv[0]);
        return 1;
    }
    Scanner s;
    string root = getenv("HOME");
    string query = argv[1];
    root += "/Music";
    vector<string> files = s.scanFiles(root, AudioMedia);
    printf("Found %ld files.\n", (long)files.size());
    return do_query(files, query);
}
