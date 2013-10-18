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
#include<sys/select.h>
#include<cerrno>
#include<cstring>
#include<unistd.h>
#include<map>
#include<memory>
#include<cassert>

using namespace std;

class ScannerDaemon {
public:
    ScannerDaemon();

    int run();

private:

    void readFiles(MediaStore &store, const string &subdir, const MediaType type);
    void addDir(const string &dir, const string &id);
    void removeDir(const string &dir);
    map<string, shared_ptr<SubtreeWatcher>> subtrees;
    map<string, shared_ptr<MediaStore>> stores;
};

ScannerDaemon::ScannerDaemon() {
    string homedir = "/home/";
    homedir += getlogin();
    string musicdir = homedir + "/Music";
    string videodir = homedir + "/Videos";

    addDir(musicdir, "home-music");
    addDir(videodir, "home-video");
}

void ScannerDaemon::addDir(const string &dir, const string &id) {
    assert(dir[0] == '/');
    assert(!id.empty());
    shared_ptr<MediaStore> ms(new MediaStore(id));
    shared_ptr<SubtreeWatcher> sw(new SubtreeWatcher(ms.get()));
    ms->pruneDeleted();
    // Fixme, only traverse once.
    readFiles(*ms.get(), dir, VideoMedia);
    readFiles(*ms.get(), dir, AudioMedia);
    sw->addDir(dir);
    subtrees[dir] = sw;
    stores[dir] = ms;
}

void ScannerDaemon::removeDir(const string &dir) {
    assert(dir[0] == '/');
    assert(subtrees.find(dir) != subtrees.end());
    assert(stores.find(dir) != stores.end());
    subtrees.erase(dir);
    stores.erase(dir);
}

void ScannerDaemon::readFiles(MediaStore &store, const string &subdir, const MediaType type) {
    Scanner s;
    vector<string> files = s.scanFiles(subdir, type);
    for(auto &i : files) {
        try {
            store.insert(MediaFile(i));
        } catch(const exception &e) {
            fprintf(stderr, "Error when indexing: %s\n", e.what());
        }
    }
}

int ScannerDaemon::run() {
    int kbdfd = STDIN_FILENO;
    while(true) {
        int maxfd = 0;
        fd_set fds;
        FD_ZERO(&fds);
        for(const auto &i: subtrees) {
            int cfd = i.second->getFd();
            if(cfd > maxfd) maxfd = cfd;
            FD_SET(cfd, &fds);
        }
        FD_SET(kbdfd, &fds);


        int rval = select(maxfd+1, &fds, nullptr, nullptr, nullptr);
        if(rval < 0) {
            string msg("Select failed: ");
            msg += strerror(errno);
            throw msg;
        }
        if(FD_ISSET(kbdfd, &fds)) {
            return 0;
        }
        for(const auto &i: subtrees) {
            i.second->pumpEvents();
        }
    }
}

int main(int argc, char **argv) {
    gst_init (&argc, &argv);
    try {
        ScannerDaemon d;
        printf("\n\nPress enter to end this program.\n\n");
        return d.run();
    } catch(string &s) {
        printf("Error: %s\n", s.c_str());
    }
    return 100;
}
