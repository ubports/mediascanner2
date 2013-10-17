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

#include"SubtreeWatcher.hh"
#include"MediaStore.hh"
#include"Scanner.hh"
#include"MediaFile.hh"

#include<cstdio>
#include<gst/gst.h>

using namespace std;

void readFiles(MediaStore &store, const string &subdir) {
    Scanner s;
    vector<string> files = s.scanFiles(subdir, AudioMedia);
    for(auto &i : files) {
        try {
            store.insert(MediaFile(i));
        } catch(const exception &e) {
            fprintf(stderr, "Error when indexing: %s\n", e.what());
        }
    }
}

int runDaemon(SubtreeWatcher &w) {
    int ifd = w.getFd();
    int kbdfd = 0;
    while(true) {

    }
}

int main(int argc, char **argv) {
    gst_init (&argc, &argv);
    try {
        MediaStore store;
        SubtreeWatcher sw(&store);
        if(argc != 2) {
            printf("%s <subdir to process>\n", argv[0]);
            return 1;
        }
        string rootdir(argv[1]);
        readFiles(store, rootdir);
        sw.addDir(rootdir);
        printf("Cache has %ld songs.\n", (long) store.size());
        return runDaemon(sw);
    } catch(string &s) {
        printf("Error: %s\n", s.c_str());
    }
    return 100;
}
