/*
 * Copyright (C) 2014 Canonical, Ltd.
 *
 * Authors:
 *    James Henstridge <james.henstridge@canonical.com>
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

#include <cstdio>
#include <memory>
#include <stdexcept>

#include <glib.h>
#include <gio/gio.h>
#include <gst/gst.h>

#include <mediascanner/MediaFile.hh>
#include "DetectedFile.hh"
#include "ExtractorBackend.hh"
#include "dbus-generated.h"
#include "dbus-marshal.hh"

using namespace mediascanner;

namespace {

const char BUS_NAME[] = "com.canonical.MediaScanner2.Extractor";
const char BUS_PATH[] = "/com/canonical/MediaScanner2/Extractor";
const char EXTRACT_ERROR[] = "com.canonical.MediaScanner2.Error.Failed";

const int DELAY = 30;

class ExtractorDaemon final {
public:
    ExtractorDaemon();
    ~ExtractorDaemon();
    void run();

private:
    void setupBus();
    void extract(const DetectedFile &file, GDBusMethodInvocation *invocation);
    void startExitTimer();
    void cancelExitTimer();

    static void busNameLostCallback(GDBusConnection *, const char *name, gpointer data);
    static gboolean handleExtractMetadata(MSExtractor *iface, GDBusMethodInvocation *invocation, const char *filename, const char *etag, const char *content_type, guint64 mtime, gint32 type, gpointer user_data);
    static gboolean handleExitTimer(gpointer user_data);

    ExtractorBackend extractor;
    std::unique_ptr<GMainLoop, void(*)(GMainLoop*)> main_loop;
    std::unique_ptr<GDBusConnection, void(*)(void*)> session_bus;
    unsigned int bus_name_id = 0;
    std::unique_ptr<MSExtractor, void(*)(void*)> iface;
    unsigned int handler_id = 0;
    unsigned int timeout_id = 0;

    int crash_after = -1;
};

}

ExtractorDaemon::ExtractorDaemon() :
    main_loop(g_main_loop_new(nullptr, FALSE), g_main_loop_unref),
    session_bus(nullptr, g_object_unref),
    iface(ms_extractor_skeleton_new(), g_object_unref) {
    const char *crash_after_env = getenv("MEDIASCANNER_EXTRACTOR_CRASH_AFTER");
    if (crash_after_env) {
        crash_after = std::stoi(crash_after_env);
    }
    setupBus();
}

ExtractorDaemon::~ExtractorDaemon() {
    if (bus_name_id != 0) {
        g_bus_unown_name(bus_name_id);
    }
    g_dbus_interface_skeleton_unexport(
        G_DBUS_INTERFACE_SKELETON(iface.get()));
    if (handler_id != 0) {
        g_signal_handler_disconnect(iface.get(), handler_id);
    }
    cancelExitTimer();
}

void ExtractorDaemon::setupBus() {
    GError *error = nullptr;
    session_bus.reset(g_bus_get_sync(G_BUS_TYPE_SESSION, nullptr, &error));
    if (!session_bus) {
        std::string errortxt(error->message);
        g_error_free(error);
        throw std::runtime_error(
            std::string("Failed to connect to session bus: ") + errortxt);
    }

    handler_id = g_signal_connect(
        iface.get(), "handle-extract-metadata",
        G_CALLBACK(&ExtractorDaemon::handleExtractMetadata), this);

    if (!g_dbus_interface_skeleton_export(
            G_DBUS_INTERFACE_SKELETON(iface.get()), session_bus.get(),
            BUS_PATH, &error)) {
        std::string errortxt(error->message);
        g_error_free(error);
        throw std::runtime_error(
            std::string("Failed to export object: ") + errortxt);
    }

    bus_name_id = g_bus_own_name_on_connection(
        session_bus.get(), BUS_NAME, static_cast<GBusNameOwnerFlags>(
            G_BUS_NAME_OWNER_FLAGS_ALLOW_REPLACEMENT |
            G_BUS_NAME_OWNER_FLAGS_REPLACE),
        nullptr, &ExtractorDaemon::busNameLostCallback, this, nullptr);
}

void ExtractorDaemon::busNameLostCallback(GDBusConnection *, const char *name,
                                          gpointer data) {
    ExtractorDaemon *daemon = reinterpret_cast<ExtractorDaemon*>(data);
    fprintf(stderr, "Exiting due to loss of control of bus name %s\n", name);
    daemon->bus_name_id = 0;
    g_main_loop_quit(daemon->main_loop.get());
}

gboolean ExtractorDaemon::handleExtractMetadata(MSExtractor *,
                                                GDBusMethodInvocation *invocation,
                                                const char *filename,
                                                const char *etag,
                                                const char *content_type,
                                                guint64 mtime,
                                                gint32 type,
                                                gpointer user_data) {
    auto d = reinterpret_cast<ExtractorDaemon*>(user_data);

    // If the environment variable was set, crash the service after
    // the requested number of extractions.
    if (d->crash_after == 0) {
        abort();
    } else if (d->crash_after > 0) {
        d->crash_after--;
    }

    DetectedFile file(filename, etag, content_type, mtime, static_cast<MediaType>(type));
    d->extract(file, invocation);
    return TRUE;
}

void ExtractorDaemon::extract(const DetectedFile &file,
                              GDBusMethodInvocation *invocation) {
    cancelExitTimer();
    try {
        MediaFile media = extractor.extract(file);
        ms_extractor_complete_extract_metadata(
            iface.get(), invocation, media_to_variant(media));
    } catch (const std::exception &e) {
        g_dbus_method_invocation_return_dbus_error(
            invocation, EXTRACT_ERROR, e.what());
    }
    startExitTimer();
}

void ExtractorDaemon::startExitTimer() {
    cancelExitTimer();
    timeout_id = g_timeout_add_seconds(
        DELAY, &ExtractorDaemon::handleExitTimer, this);
}

void ExtractorDaemon::cancelExitTimer() {
    if (timeout_id != 0) {
        g_source_remove(timeout_id);
        timeout_id = 0;
    }
}

gboolean ExtractorDaemon::handleExitTimer(gpointer user_data) {
    auto d = reinterpret_cast<ExtractorDaemon*>(user_data);
    fprintf(stderr, "Exiting due to inactivity\n");
    g_main_loop_quit(d->main_loop.get());
    d->timeout_id = 0;
    return G_SOURCE_REMOVE;
}

void ExtractorDaemon::run() {
    g_main_loop_run(main_loop.get());
}


int main(int argc, char **argv) {
    gst_init(&argc, &argv);
    try {
        ExtractorDaemon d;
        d.run();
    } catch (const std::exception &e) {
        fprintf(stderr, "Error: %s\n", e.what());
        return 1;
    }
    return 0;
}
