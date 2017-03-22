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
#include "../extractor/MetadataExtractor.hh"
#include "MountWatcher.hh"
#include "InvalidationSender.hh"
#include "VolumeManager.hh"

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
    static gboolean signalCallback(gpointer data);
    static void busNameLostCallback(GDBusConnection *connection, const char *name, gpointer data);
    void mountEvent(const MountWatcher::Info &info);

    unique_ptr<MountWatcher> mount_watcher;
    unsigned int sigint_id = 0, sigterm_id = 0;
    string cachedir;
    unique_ptr<MediaStore> store;
    unique_ptr<MetadataExtractor> extractor;
    InvalidationSender invalidator;
    unique_ptr<VolumeManager> volumes;
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
    volumes.reset(new VolumeManager(*store, *extractor, invalidator));

    setupMountWatcher();

    const char *musicdir = g_get_user_special_dir(G_USER_DIRECTORY_MUSIC);
    const char *videodir = g_get_user_special_dir(G_USER_DIRECTORY_VIDEOS);
    const char *picturesdir = g_get_user_special_dir(G_USER_DIRECTORY_PICTURES);
    const char *homedir = g_get_home_dir();

    // According to XDG specification, when one of the special dirs is missing
    // it falls back to home directory. This would mean scanning the entire home
    // directory. This is probably not what people want so skip if this is the case.
    if (musicdir && !is_same_directory(musicdir, homedir))
        volumes->queueAddVolume(musicdir);

    if (videodir && !is_same_directory(videodir, homedir))
        volumes->queueAddVolume(videodir);

    if (picturesdir && !is_same_directory(picturesdir, homedir))
        volumes->queueAddVolume(picturesdir);

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
    if (info.is_mounted) {
        printf("Volume %s was mounted.\n", info.mount_point.c_str());
        if (info.mount_point.substr(0, 6) == "/media") {
            volumes->queueAddVolume(info.mount_point);
        }
    } else {
        printf("Volume %s was unmounted.\n", info.mount_point.c_str());
        volumes->queueRemoveVolume(info.mount_point);
    }
}

static void validate_desktop() {
    // Only set manually
    const gchar *ms_run_env = g_getenv("MEDIASCANNER_RUN");
    if (g_strcmp0(ms_run_env, "1") == 0)
        return;

    // Only set properly in 17.04 and onward
    const gchar *desktop_env = g_getenv("XDG_CURRENT_DESKTOP");
    if (g_regex_match_simple("(^|:)Unity8(:|$)", desktop_env,
                             (GRegexCompileFlags)0, (GRegexMatchFlags)0))
        return;

    // Shouldn't rely on this, but we only need to use it for 16.04 - 17.04
    const gchar *session_env = g_getenv("XDG_SESSION_DESKTOP");
    if (g_strcmp0("unity8", session_env) == 0)
        return;

    // We shouldn't run if we weren't asked for; we can confuse some desktops
    // (like unity7) with our scanning of mounted drives and the like.
    printf("Mediascanner service not starting due to unsupported desktop environment.\n");
    printf("Set MEDIASCANNER_RUN=1 to override this.\n");
    exit(0);
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
    validate_desktop();
    print_banner();

    try {
        ScannerDaemon d;
        return d.run();
    } catch(string &s) {
        printf("Error: %s\n", s.c_str());
    }
    return 100;
}
