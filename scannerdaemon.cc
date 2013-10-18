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
#include<sys/stat.h>
#include<sys/inotify.h>
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

    void setupMountWatcher();
    void readFiles(MediaStore &store, const string &subdir, const MediaType type);
    void addDir(const string &dir, const string &id);
    void removeDir(const string &dir);
    void pumpEvents();
    void addMountedVolumes();

    int mountfd;
    string mountDir;
    map<string, shared_ptr<SubtreeWatcher>> subtrees;
    map<string, shared_ptr<MediaStore>> stores;
};

ScannerDaemon::ScannerDaemon() {
    string homedir = "/home/";
    homedir += getlogin();
    string musicdir = homedir + "/Music";
    string videodir = homedir + "/Videos";

    mountDir = "/home/jpakkane/workspace/scantest/build";
    setupMountWatcher();
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
    addMountedVolumes();
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
        FD_SET(mountfd, &fds);
        if(mountfd > maxfd) maxfd = mountfd;

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
        pumpEvents();
    }
    return 99;
}

void ScannerDaemon::setupMountWatcher() {
    mountfd = inotify_init();
    if(mountfd < 0) {
        string msg("Could not init inotify: ");
        msg += strerror(errno);
        throw msg;
    }
    int wd = inotify_add_watch(mountfd, mountDir.c_str(),
            IN_CREATE |  IN_DELETE | IN_ONLYDIR);
    if(wd == -1) {
        string msg("Could not create inotify watch object: ");
        msg += strerror(errno);
        throw msg;
    }
}

void ScannerDaemon::pumpEvents() {
    const int BUFSIZE= 4096;
    char buf[BUFSIZE];
    while(true) {
        fd_set reads;
        FD_ZERO(&reads);
        FD_SET(mountfd, &reads);
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 0;
        if(select(mountfd+1, &reads, nullptr, nullptr, &timeout) <= 0) {
            return;
        }
        ssize_t num_read;
        num_read = read(mountfd, buf, BUFSIZE);
        if(num_read == 0) {
            printf("Inotify returned 0.\n");
            break;
        }
        if(num_read == -1) {
            printf("Read error.\n");
            break;
        }
        for(char *p = buf; p < buf + num_read;) {
            struct inotify_event *event = (struct inotify_event *) p;
            string directory = mountDir;
            string filename(event->name);
            string abspath = directory + '/' + filename;
            string mountId = "mount-" + filename;
            struct stat statbuf;
            stat(abspath.c_str(), &statbuf);
            if(S_ISDIR(statbuf.st_mode)) {
                if(event->mask & IN_CREATE) {
                    printf("Volume %s was mounted.\n", abspath.c_str());
                    addDir(abspath, mountId);
                } else if(event->mask & IN_DELETE){
                    printf("Volume %s was unmounted.\n", abspath.c_str());
                    removeDir(abspath);
                }
            }
            p += sizeof(struct inotify_event) + event->len;
        }
    }
}

void ScannerDaemon::addMountedVolumes() {
    unique_ptr<DIR, int(*)(DIR*)> dir(opendir(mountDir.c_str()), closedir);
    if(!dir) {
        return;
    }
    struct dirent* curloc;
    while((curloc = readdir(dir.get())) ) {
        struct stat statbuf;
        string fname = curloc->d_name;
        if(fname == "." || fname == "..") // Maybe ignore all entries starting with a period?
            continue;
        string fullpath = mountDir + "/" + fname;
        string mountId = "mount-" + fname;
        stat(fullpath.c_str(), &statbuf);
        if(S_ISDIR(statbuf.st_mode)) {
            addDir(fullpath, mountId);
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
