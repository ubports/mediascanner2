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
#include "metadataextractor.hh"

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
    ~ScannerDaemon();
    int run();

private:

    void setupMountWatcher();
    void readFiles(MediaStore &store, const string &subdir, const MediaType type);
    void addDir(const string &dir);
    void removeDir(const string &dir);
    void pumpEvents();
    void addMountedVolumes();

    int mountfd;
    string mountDir;
    string cachedir;
    unique_ptr<MediaStore> store;
    unique_ptr<MetadataExtractor> extractor;
    map<string, unique_ptr<SubtreeWatcher>> subtrees;
};

ScannerDaemon::ScannerDaemon() {
    string homedir = "/home/";
    homedir += getlogin();
    string musicdir = homedir + "/Music";
    string videodir = homedir + "/Videos";
    char *env_cachedir = getenv("MEDIASCANNER_CACHEDIR");
    if(env_cachedir) {
        cachedir = env_cachedir;
    } else {
        cachedir = homedir + "/.cache/mediascanner-test";
    }
    int ec;
    errno = 0;
    ec = mkdir(cachedir.c_str(), S_IRUSR | S_IWUSR | S_IXUSR);
    if(ec < 0 && errno != EEXIST) {
        string msg("Could not create cache dir: ");
        msg += strerror(errno);
        throw runtime_error(msg);
    }
    mountDir = string("/media/") + getlogin();
    unique_ptr<MediaStore> tmp(new MediaStore(cachedir + "/mediastore.db", MS_READ_WRITE, "/media/"));
    store = move(tmp);
    extractor.reset(new MetadataExtractor());
    setupMountWatcher();
    addMountedVolumes();
    addDir(musicdir);
    addDir(videodir);
}

ScannerDaemon::~ScannerDaemon() {
    close(mountfd);
}

void ScannerDaemon::addDir(const string &dir) {
    assert(dir[0] == '/');
    if(subtrees.find(dir) != subtrees.end()) {
        return;
    }
    unique_ptr<SubtreeWatcher> sw(new SubtreeWatcher(*store.get(), *extractor.get()));
    store->restoreItems(dir);
    store->pruneDeleted();
    // Fixme, only traverse once.
    readFiles(*store.get(), dir, VideoMedia);
    readFiles(*store.get(), dir, AudioMedia);
    sw->addDir(dir);
    subtrees[dir] = move(sw);
}

void ScannerDaemon::removeDir(const string &dir) {
    assert(dir[0] == '/');
    assert(subtrees.find(dir) != subtrees.end());
    subtrees.erase(dir);

}

void ScannerDaemon::readFiles(MediaStore &store, const string &subdir, const MediaType type) {
    Scanner s;
    vector<string> files = s.scanFiles(subdir, type);
    for(auto &i : files) {
        try {
            store.insert(extractor->extract(i));
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
            throw runtime_error(msg);
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
        throw runtime_error(msg);
    }
    int wd = inotify_add_watch(mountfd, mountDir.c_str(),
            IN_CREATE |  IN_DELETE | IN_ONLYDIR);
    if(wd == -1) {
        if (errno == ENOENT) {
            printf("Mount directory does not exist\n");
            return;
        }
        string msg("Could not create inotify watch object: ");
        msg += strerror(errno);
        throw runtime_error(msg);
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
            struct stat statbuf;
            stat(abspath.c_str(), &statbuf);
            if(S_ISDIR(statbuf.st_mode)) {
                if(event->mask & IN_CREATE) {
                    printf("Volume %s was mounted.\n", abspath.c_str());
                    addDir(abspath);
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
    unique_ptr<struct dirent, void(*)(void*)> entry((dirent*)malloc(sizeof(dirent) + NAME_MAX),
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
            addDir(fullpath);
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
