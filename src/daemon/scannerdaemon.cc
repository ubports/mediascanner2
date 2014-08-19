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
#include<dirent.h>

#include<glib.h>
#include<glib-unix.h>
#include<gio/gio.h>
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

static const char BUS_NAME[] = "com.canonical.MediaScanner2.Daemon";

class ScannerDaemon final {
public:
    ScannerDaemon();
    ~ScannerDaemon();
    int run();

private:

    void setupBus();
    void setupSignals();
    void setupMountWatcher();
    void readFiles(MediaStore &store, const string &subdir, const MediaType type);
    void addDir(const string &dir);
    void removeDir(const string &dir);
    static gboolean sourceCallback(int, GIOCondition, gpointer data);
    static gboolean signalCallback(gpointer data);
    static void busNameLostCallback(GDBusConnection *connection, const char *name, gpointer data);
    void processEvents();
    bool addMountedVolumes();
    bool mountEvent(const string& abspath, struct inotify_event* event);
    bool preMountEvent(const string& abspath, struct inotify_event* event);

    int mountfd;
    unique_ptr<GSource,void(*)(GSource*)> mount_source;
    unsigned int sigint_id = 0, sigterm_id = 0;
    string mountDir;
    string cachedir;
    unique_ptr<MediaStore> store;
    unique_ptr<MetadataExtractor> extractor;
    map<string, unique_ptr<SubtreeWatcher>> subtrees;
    InvalidationSender invalidator;
    unique_ptr<GMainLoop,void(*)(GMainLoop*)> main_loop;
    unique_ptr<GDBusConnection,void(*)(void*)> session_bus;
    unsigned int bus_name_id = 0;
    // Under some circumstances the directory /media/username does not
    // exist when Mediascanner is first run. In this case we need to track
    // when it appears and then change into tracking changes there.
    // /media/username is never deleted during the life cycle of Mediascanner.
    bool mountdir_exists;
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

static void source_destroyer(GSource *s) {
    if(s) {
        g_source_destroy(s);
        g_source_unref(s);
    }
}

ScannerDaemon::ScannerDaemon() :
    mount_source(nullptr, source_destroyer),
    main_loop(g_main_loop_new(nullptr, FALSE), g_main_loop_unref),
    session_bus(nullptr, g_object_unref) {
    setupBus();
    mountDir = string("/media/") + getCurrentUser();
    auto dir = opendir(mountDir.c_str());
    mountdir_exists = dir ? true : false;
    if(dir)
        closedir(dir);
    store.reset(new MediaStore(MS_READ_WRITE, "/media/"));
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
    if (bus_name_id != 0) {
        g_bus_unown_name(bus_name_id);
    }
    close(mountfd);
}

void ScannerDaemon::busNameLostCallback(GDBusConnection *, const char *name,
                                        gpointer data) {
    ScannerDaemon *daemon = static_cast<ScannerDaemon*>(data);
    fprintf(stderr, "Exiting due to loss of control of bus name %s\n", name);
    daemon->bus_name_id = 0;
    g_main_loop_quit(daemon->main_loop.get());
}

void ScannerDaemon::setupBus() {
    GError *error = nullptr;
    session_bus.reset(g_bus_get_sync(G_BUS_TYPE_SESSION, nullptr, &error));
    if (!session_bus) {
        string errortxt(error->message);
        g_error_free(error);
        string msg = "Failed to connect to session bus: ";
        msg += errortxt;
        throw runtime_error(msg);
    }
    invalidator.setBus(session_bus.get());

    bus_name_id = g_bus_own_name_on_connection(
        session_bus.get(), BUS_NAME, static_cast<GBusNameOwnerFlags>(
            G_BUS_NAME_OWNER_FLAGS_ALLOW_REPLACEMENT |
            G_BUS_NAME_OWNER_FLAGS_REPLACE),
        nullptr, &ScannerDaemon::busNameLostCallback, this, nullptr);
}

gboolean ScannerDaemon::signalCallback(gpointer data) {
    ScannerDaemon *daemon = static_cast<ScannerDaemon*>(data);
    g_main_loop_quit(daemon->main_loop.get());
    return G_SOURCE_CONTINUE;
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
    if(subtrees.find(dir) == subtrees.end())
        return;
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
    return G_SOURCE_CONTINUE;
}

void ScannerDaemon::setupMountWatcher() {
    mountfd = inotify_init();
    if(mountfd < 0) {
        string msg("Could not init inotify: ");
        msg += strerror(errno);
        throw runtime_error(msg);
    }
    std::string watched_dir = mountdir_exists ? mountDir : "/media";
    auto watch_flags = mountdir_exists ? (IN_CREATE |  IN_DELETE | IN_ONLYDIR) : (IN_CREATE | IN_ONLYDIR);
    int wd = inotify_add_watch(mountfd, watched_dir.c_str(), watch_flags);
    if(wd == -1) {
        if (errno == ENOENT) {
            printf("Mount directory does not exist\n");
            return;
        }
        string msg("Could not create inotify watch object: ");
        msg += strerror(errno);
        throw runtime_error(msg);
    }

    if(!mountdir_exists) {
        printf("%s does not exist yet, watching /media until it appears.\n", mountDir.c_str());
    }
    mount_source.reset(g_unix_fd_source_new(mountfd, G_IO_IN));
    g_source_set_callback(mount_source.get(), reinterpret_cast<GSourceFunc>(&ScannerDaemon::sourceCallback), this, nullptr);
    g_source_attach(mount_source.get(), nullptr);
}

bool ScannerDaemon::mountEvent(const string& abspath, struct inotify_event* event) {
    bool changed = false;
    if (event->mask & IN_CREATE) {
        printf("Volume %s was mounted.\n", abspath.c_str());
        addDir(abspath);
        changed = true;
    } else if (event->mask & IN_DELETE) {
        printf("Volume %s was unmounted.\n", abspath.c_str());
        if (subtrees.find(abspath) != subtrees.end()) {
            removeDir(abspath);
            changed = true;
        } else {
            // This volume was not tracked because it looked rootlike,
            // or maybe it got lost in an inotify event flood.
            // Thus we don't need to do anything.
        }
    }
    return changed;
}

bool ScannerDaemon::preMountEvent(const string& abspath, struct inotify_event* event) {
    bool changed = false;
    if(mountdir_exists) {
        return false; // There may have been multiple events in the queue so ignore later ones.
    }
    if (event->mask & IN_CREATE) {
        if(abspath == mountDir) {
            printf("Media mount location %s was created.\n", abspath.c_str());
            mountdir_exists = true;
            close(mountfd);
            mount_source.reset(nullptr);
            setupMountWatcher();
            changed = addMountedVolumes();
        }
    }
    return changed;
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
        string directory = mountdir_exists ? mountDir : "/media";
        string filename(event->name);
        string abspath = directory + '/' + filename;
        // We only get events for directories as per the inotify flags.
        if(mountdir_exists) {
            changed = mountEvent(abspath, event);
        } else {
            changed = preMountEvent(abspath, event);
        }
        p += sizeof(struct inotify_event) + event->len;
    }
    if (changed) {
        invalidator.invalidate();
    }
}

bool ScannerDaemon::addMountedVolumes() {
    bool changed = false;
    unique_ptr<DIR, int(*)(DIR*)> dir(opendir(mountDir.c_str()), closedir);
    if(!dir) {
        return changed;
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
            changed = true;
            addDir(fullpath);
        }
    }
    return changed;
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
