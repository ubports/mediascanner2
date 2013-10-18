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

using namespace std;

void readFiles(MediaStore &store, const string &subdir, const MediaType type) {
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

int runDaemon(SubtreeWatcher &w) {
    int ifd = w.getFd();
    int kbdfd = STDIN_FILENO;
    while(true) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(ifd, &fds);
        FD_SET(kbdfd, &fds);
        int rval = select(ifd+1, &fds, nullptr, nullptr, nullptr);
        if(rval < 0) {
            string msg("Select failed: ");
            msg += strerror(errno);
            throw msg;
        }
        if(FD_ISSET(kbdfd, &fds)) {
            return 0;
        }
        w.pumpEvents();
    }
}

class ScannerDaemon {
public:
    ScannerDaemon();

    int run();

private:
    map<string, shared_ptr<SubtreeWatcher>> subtrees;
    map<string, shared_ptr<MediaStore>> stores;
};

ScannerDaemon::ScannerDaemon() {
    string homedir = "/home/";
    homedir += getlogin();
    string musicdir = homedir + "/Music";
    string videodir = homedir + "/Videos";
    shared_ptr<MediaStore> ms(new MediaStore());
    shared_ptr<SubtreeWatcher> sw(new SubtreeWatcher(ms.get()));
    ms->pruneDeleted();
    // FIXME, only traverse tree once.
    readFiles(*ms.get(), musicdir, AudioMedia);
    readFiles(*ms.get(), musicdir, VideoMedia);
    sw->addDir(musicdir);
    subtrees[musicdir] = sw;
    stores[musicdir] = ms;
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
