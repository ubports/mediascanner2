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
#include<glib-unix.h>
#include<gst/gst.h>

#include "../mediascanner/MediaFile.hh"
#include "../mediascanner/MediaStore.hh"
#include "MetadataExtractor.hh"
#include "SubtreeWatcher.hh"
#include "Scanner.hh"
#include "InvalidationSender.hh"
#include "../mediascanner/internal/utils.hh"

using namespace std;

using namespace mediascanner;

class ScannerDaemon final {
public:
    ScannerDaemon();
    ~ScannerDaemon();
    int run();

private:

    void setupSignals();
    void setupMountWatcher();
    void readFiles(MediaStore &store, const string &subdir, const MediaType type);
    void addDir(const string &dir);
    void removeDir(const string &dir);
    static gboolean sourceCallback(int, GIOCondition, gpointer data);
    static gboolean signalCallback(gpointer data);
    void processEvents();
    void addMountedVolumes();

    int mountfd;
    unique_ptr<GSource,void(*)(GSource*)> mount_source;
    int sigint_id, sigterm_id;
    string mountDir;
    string cachedir;
    unique_ptr<MediaStore> store;
    unique_ptr<MetadataExtractor> extractor;
    map<string, unique_ptr<SubtreeWatcher>> subtrees;
    InvalidationSender invalidator;
    unique_ptr<GMainLoop,void(*)(GMainLoop*)> main_loop;
};

static std::string getCurrentUser() {
    int uid = geteuid();
    struct passwd *pwd = getpwuid(uid);
    if (pwd == nullptr) {
            string msg("Could not look up user name: ");
            msg += strerror(errno);
            throw runtime_error(msg);
    }
    return pwd->pw_name;
}

ScannerDaemon::ScannerDaemon() :
    mount_source(nullptr, g_source_unref), sigint_id(0), sigterm_id(0),
    main_loop(g_main_loop_new(nullptr, FALSE), g_main_loop_unref) {
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

    const char *picturesdir = g_get_user_special_dir(G_USER_DIRECTORY_PICTURES);
    if (picturesdir)
        addDir(picturesdir);

    // In case someone opened the db file before we could populate it.
    invalidator.invalidate();
    // This is at the end because the initial scan may take a while
    // and is not interruptible but we want the process to die if it
    // gets a SIGINT or the like.
    setupSignals();
}

ScannerDaemon::~ScannerDaemon() {
    if (sigint_id != 0) {
        g_source_remove(sigint_id);
    }
    if (sigterm_id != 0) {
        g_source_remove(sigterm_id);
    }
    if (mount_source) {
        g_source_destroy(mount_source.get());
    }
    close(mountfd);
}

gboolean ScannerDaemon::signalCallback(gpointer data) {
    ScannerDaemon *daemon = static_cast<ScannerDaemon*>(data);
    g_main_loop_quit(daemon->main_loop.get());
    return TRUE;
}

void ScannerDaemon::setupSignals() {
    sigint_id = g_unix_signal_add(SIGINT, &ScannerDaemon::signalCallback, this);
    sigterm_id = g_unix_signal_add(SIGTERM, &ScannerDaemon::signalCallback, this);
}

void ScannerDaemon::addDir(const string &dir) {
    assert(dir[0] == '/');
    if(subtrees.find(dir) != subtrees.end()) {
        return;
    }
    if(is_rootlike(dir)) {
        fprintf(stderr, "Directory %s looks like a top level root directory, skipping it (%s).\n",
                dir.c_str(), __PRETTY_FUNCTION__);
        return;
    }
    if(is_optical_disc(dir)) {
        fprintf(stderr, "Directory %s looks like an optical disc, skipping it.\n", dir.c_str());
        return;
    }
    if(has_scanblock(dir)) {
        fprintf(stderr, "Directory %s has a scan block file, skipping it.\n", dir.c_str());
        return;
    }
    unique_ptr<SubtreeWatcher> sw(new SubtreeWatcher(*store.get(), *extractor.get(), invalidator));
    store->restoreItems(dir);
    store->pruneDeleted();
    readFiles(*store.get(), dir, AllMedia);
    sw->addDir(dir);
    subtrees[dir] = move(sw);
}

void ScannerDaemon::removeDir(const string &dir) {
    assert(dir[0] == '/');
    assert(subtrees.find(dir) != subtrees.end());
    store->archiveItems(dir);
    subtrees.erase(dir);
}

void ScannerDaemon::readFiles(MediaStore &store, const string &subdir, const MediaType type) {
    Scanner s(extractor.get(), subdir, type);
    while(true) {
        try {
            auto d = s.next();
            // If the file is unchanged, skip it.
            if (d.etag == store.getETag(d.filename))
                continue;
            try {
                store.insert(extractor->extract(d));
            } catch(const exception &e) {
                fprintf(stderr, "Error when indexing: %s\n", e.what());
            }
        } catch(const StopIteration &stop) {
            return;
        }
    }
}

int ScannerDaemon::run() {
    g_main_loop_run(main_loop.get());
    return 99;
}

gboolean ScannerDaemon::sourceCallback(int, GIOCondition, gpointer data) {
    ScannerDaemon *daemon = static_cast<ScannerDaemon*>(data);
    daemon->processEvents();
    return TRUE;
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

    mount_source.reset(g_unix_fd_source_new(mountfd, G_IO_IN));
    g_source_set_callback(mount_source.get(), reinterpret_cast<GSourceFunc>(&ScannerDaemon::sourceCallback), this, nullptr);
    g_source_attach(mount_source.get(), nullptr);
}

void ScannerDaemon::processEvents() {
    const int BUFSIZE= 4096;
    char buf[BUFSIZE];
    bool changed = false;
    ssize_t num_read;
    num_read = read(mountfd, buf, BUFSIZE);
    if(num_read == 0) {
        printf("Inotify returned 0.\n");
        return;
    }
    if(num_read == -1) {
        printf("Read error.\n");
        return;
    }
    for(char *p = buf; p < buf + num_read;) {
        struct inotify_event *event = (struct inotify_event *) p;
        string directory = mountDir;
        string filename(event->name);
        string abspath = directory + '/' + filename;
        struct stat statbuf;
        lstat(abspath.c_str(), &statbuf);
        if(S_ISDIR(statbuf.st_mode)) {
            if(event->mask & IN_CREATE) {
                printf("Volume %s was mounted.\n", abspath.c_str());
                addDir(abspath);
                changed = true;
            } else if(event->mask & IN_DELETE){
                printf("Volume %s was unmounted.\n", abspath.c_str());
                if(subtrees.find(abspath) != subtrees.end()) {
                    removeDir(abspath);
                    changed = true;
                } else {
                    // This volume was not tracked because it looked rootlike.
                    // Thus we don't need to do anything.
                }
            }
        }
        p += sizeof(struct inotify_event) + event->len;
    }
    if (changed) {
        invalidator.invalidate();
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
        lstat(fullpath.c_str(), &statbuf);
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
