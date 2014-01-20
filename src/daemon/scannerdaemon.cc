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

#include<pwd.h>
#include<sys/select.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/inotify.h>
#include<unistd.h>
#include<cstdio>
#include<cerrno>
#include<cstring>
#include<map>
#include<memory>
#include<cassert>

#include<glib.h>
#include<gst/gst.h>

#include "../mediascanner/MediaFile.hh"
#include "../mediascanner/MediaStore.hh"
#include "MetadataExtractor.hh"
#include "SubtreeWatcher.hh"
#include "Scanner.hh"
#include "InvalidationSender.hh"

using namespace std;

class ScannerDaemon final {
public:
    ScannerDaemon();
    ~ScannerDaemon();
    int run();

private:

    void setupMountWatcher();
    void readFiles(MediaStore &store, const string &subdir, const MediaType type);
    void addDir(const string &dir);
    void removeDir(const string &dir);
    bool pumpEvents();
    void addMountedVolumes();

    int mountfd;
    string mountDir;
    string cachedir;
    unique_ptr<MediaStore> store;
    unique_ptr<MetadataExtractor> extractor;
    map<string, unique_ptr<SubtreeWatcher>> subtrees;
    InvalidationSender invalidator;
};

static std::string getCurrentUser() {
    int uid = geteuid();
    struct passwd *pwd = getpwuid(uid);
    if (pwd == NULL) {
            string msg("Could not look up user name: ");
            msg += strerror(errno);
            throw runtime_error(msg);
    }
    return pwd->pw_name;
}

ScannerDaemon::ScannerDaemon() {
    mountDir = string("/media/") + getCurrentUser();
    unique_ptr<MediaStore> tmp(new MediaStore(MS_READ_WRITE, "/media/"));
    store = move(tmp);
    extractor.reset(new MetadataExtractor());
    setupMountWatcher();
    addMountedVolumes();

    const char *musicdir = g_get_user_special_dir(G_USER_DIRECTORY_MUSIC);
    if (musicdir)
        addDir(musicdir);

    const char *videodir = g_get_user_special_dir(G_USER_DIRECTORY_VIDEOS);
    if (videodir)
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
    readFiles(*store.get(), dir, AllMedia);
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
    vector<DetectedFile> files = s.scanFiles(extractor.get(), subdir, type);
    for(auto &d : files) {
        // If the file is unchanged, skip it.
        if (d.etag == store.getETag(d.filename))
            continue;
        try {
            store.insert(extractor->extract(d));
        } catch(const exception &e) {
            fprintf(stderr, "Error when indexing: %s\n", e.what());
        }
    }
}

int ScannerDaemon::run() {
    while(true) {
        int maxfd = 0;
        bool changed = false;
        fd_set fds;
        FD_ZERO(&fds);
        for(const auto &i: subtrees) {
            int cfd = i.second->getFd();
            if(cfd > maxfd) maxfd = cfd;
            FD_SET(cfd, &fds);
        }
        FD_SET(mountfd, &fds);
        if(mountfd > maxfd) maxfd = mountfd;

        int rval = select(maxfd+1, &fds, nullptr, nullptr, nullptr);
        if(rval < 0) {
            string msg("Select failed: ");
            msg += strerror(errno);
            throw runtime_error(msg);
        }
        for(const auto &i: subtrees) {
            changed = i.second->pumpEvents() || changed;
        }
        changed = pumpEvents() || changed;
        if(changed) {
            invalidator.invalidate();
        }
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

bool ScannerDaemon::pumpEvents() {
    const int BUFSIZE= 4096;
    char buf[BUFSIZE];
    bool changed = false;
    while(true) {
        fd_set reads;
        FD_ZERO(&reads);
        FD_SET(mountfd, &reads);
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 0;
        if(select(mountfd+1, &reads, nullptr, nullptr, &timeout) <= 0) {
            break;
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
                    changed = true;
                } else if(event->mask & IN_DELETE){
                    printf("Volume %s was unmounted.\n", abspath.c_str());
                    removeDir(abspath);
                    changed = true;
                }
            }
            p += sizeof(struct inotify_event) + event->len;
        }
    }
    return changed;
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
        return d.run();
    } catch(string &s) {
        printf("Error: %s\n", s.c_str());
    }
    return 100;
}
