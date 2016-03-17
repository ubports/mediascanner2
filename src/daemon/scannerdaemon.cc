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

#include<cassert>
#include<cstdio>
#include<cstring>
#include<ctime>
#include<map>
#include<memory>

#include<sys/types.h>
#include<sys/stat.h>
#include<glib.h>
#include<glib-unix.h>
#include<gio/gio.h>

#include "../mediascanner/MediaFile.hh"
#include "../mediascanner/MediaStore.hh"
#include "../extractor/DetectedFile.hh"
#include "../extractor/MetadataExtractor.hh"
#include "MountWatcher.hh"
#include "SubtreeWatcher.hh"
#include "Scanner.hh"
#include "InvalidationSender.hh"
#include "../mediascanner/internal/utils.hh"

using namespace std;

using namespace mediascanner;

namespace {

bool is_same_directory(const char *dir1, const char *dir2) {
    struct stat s1, s2;
    if(stat(dir1, &s1) != 0) {
        return false;
    }
    if(stat(dir2, &s2) != 0) {
        return false;
    }
    return s1.st_dev == s2.st_dev && s1.st_ino == s2.st_ino;
}

}

static const char BUS_NAME[] = "com.canonical.MediaScanner2.Daemon";
static const unsigned int INVALIDATE_DELAY = 1;


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
    static gboolean signalCallback(gpointer data);
    static void busNameLostCallback(GDBusConnection *connection, const char *name, gpointer data);
    void mountEvent(const MountWatcher::Info &info);

    unique_ptr<MountWatcher> mount_watcher;
    unsigned int sigint_id = 0, sigterm_id = 0;
    string cachedir;
    unique_ptr<MediaStore> store;
    unique_ptr<MetadataExtractor> extractor;
    map<string, unique_ptr<SubtreeWatcher>> subtrees;
    InvalidationSender invalidator;
    unique_ptr<GMainLoop,void(*)(GMainLoop*)> main_loop;
    unique_ptr<GDBusConnection,void(*)(void*)> session_bus;
    unsigned int bus_name_id = 0;
};

ScannerDaemon::ScannerDaemon() :
    main_loop(g_main_loop_new(nullptr, FALSE), g_main_loop_unref),
    session_bus(nullptr, g_object_unref) {
    setupBus();
    store.reset(new MediaStore(MS_READ_WRITE, "/media/"));
    extractor.reset(new MetadataExtractor(session_bus.get()));

    setupMountWatcher();

    const char *musicdir = g_get_user_special_dir(G_USER_DIRECTORY_MUSIC);
    const char *videodir = g_get_user_special_dir(G_USER_DIRECTORY_VIDEOS);
    const char *picturesdir = g_get_user_special_dir(G_USER_DIRECTORY_PICTURES);
    const char *homedir = g_get_home_dir();

    // According to XDG specification, when one of the special dirs is missing
    // it falls back to home directory. This would mean scanning the entire home
    // directory. This is probably not what people want so skip if this is the case.
    if (musicdir && !is_same_directory(musicdir, homedir))
        addDir(musicdir);

    if (videodir && !is_same_directory(videodir, homedir))
        addDir(videodir);

    if (picturesdir && !is_same_directory(picturesdir, homedir))
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
    invalidator.setDelay(INVALIDATE_DELAY);

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
    MediaStoreTransaction txn = store.beginTransaction();
    const int update_interval = 10; // How often to send invalidations.
    struct timespec previous_update, current_time;
    clock_gettime(CLOCK_MONOTONIC, &previous_update);
    previous_update.tv_sec -= update_interval/2; // Send the first update sooner for better visual appeal.
    while(true) {
        try {
            auto d = s.next();
            clock_gettime(CLOCK_MONOTONIC, &current_time);
            while(g_main_context_pending(g_main_context_default())) {
                g_main_context_iteration(g_main_context_default(), FALSE);
            }
            if(current_time.tv_sec - previous_update.tv_sec >= update_interval) {
                txn.commit();
                invalidator.invalidate();
                previous_update = current_time;
            }
            // If the file is broken or unchanged, use fallback.
            if (store.is_broken_file(d.filename, d.etag)) {
                fprintf(stderr, "Using fallback data for unscannable file %s.\n", d.filename.c_str());
                store.insert(extractor->fallback_extract(d));
                continue;
            }
            if(d.etag == store.getETag(d.filename))
                continue;

            try {
                store.insert_broken_file(d.filename, d.etag);
                MediaFile media;
                try {
                    media = extractor->extract(d);
                } catch (const runtime_error &e) {
                    fprintf(stderr, "Error extracting from '%s': %s\n",
                            d.filename.c_str(), e.what());
                    media = extractor->fallback_extract(d);
                }
                store.insert(std::move(media));
            } catch(const exception &e) {
                fprintf(stderr, "Error when indexing: %s\n", e.what());
            }
        } catch(const StopIteration &stop) {
            break;
        }
    }
    txn.commit();
}

int ScannerDaemon::run() {
    g_main_loop_run(main_loop.get());
    return 99;
}

void ScannerDaemon::setupMountWatcher() {
    try {
        using namespace std::placeholders;
        mount_watcher.reset(
            new MountWatcher(std::bind(&ScannerDaemon::mountEvent, this, _1)));
    } catch (const std::runtime_error &e) {
        fprintf(stderr, "Failed to connect to udisksd: %s\n", e.what());
        fprintf(stderr, "Removable media support disabled\n");
        return;
    }
}

void ScannerDaemon::mountEvent(const MountWatcher::Info& info) {
    bool changed = false;
    if (info.is_mounted) {
        printf("Volume %s was mounted.\n", info.mount_point.c_str());
        if (info.mount_point.substr(0, 6) == "/media") {
            addDir(info.mount_point);
            changed = true;
        }
    } else {
        printf("Volume %s was unmounted.\n", info.mount_point.c_str());
        if (subtrees.find(info.mount_point) != subtrees.end()) {
            removeDir(info.mount_point);
            changed = true;
        } else {
            // This volume was not tracked because it looked rootlike.
            // Thus we don't need to do anything.
        }
    }
    if (changed) {
        invalidator.invalidate();
    }
}

static void print_banner() {
    char timestr[200];
    time_t t;
    struct tm *tmp;

    t = time(NULL);
    tmp = localtime(&t);
    if (tmp == NULL) {
        printf("\nMediascanner service starting.\n\n");
        return;
    }

    if (strftime(timestr, sizeof(timestr), "%Y-%m-%d %l:%M:%S", tmp) == 0) {
        printf("\nMediascanner service starting.\n\n");
        return;
    }

    printf("\nMediascanner service starting at %s.\n\n", timestr);
}

int main(int /*argc*/, char **/*argv*/) {
    print_banner();

    try {
        ScannerDaemon d;
        return d.run();
    } catch(string &s) {
        printf("Error: %s\n", s.c_str());
    }
    return 100;
}
