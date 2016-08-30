/*
 * Copyright (C) 2014 Canonical, Ltd.
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

#include"InvalidationSender.hh"
#include<string>
#include<cstdlib>
#include<cstdio>
#include <glib.h>
#include <gio/gio.h>

using namespace std;

// timer delay in seconds
static const char SCOPES_DBUS_IFACE[] = "com.canonical.unity.scopes";
static const char SCOPES_DBUS_PATH[] = "/com/canonical/unity/scopes";
static const char SCOPES_INVALIDATE_RESULTS[] = "InvalidateResults";

namespace mediascanner {

InvalidationSender::InvalidationSender() :
    bus(nullptr, g_object_unref) {
}

InvalidationSender::~InvalidationSender() {
    if (timeout_id != 0) {
        g_source_remove(timeout_id);
    }
}

void InvalidationSender::setBus(GDBusConnection *bus) {
    this->bus.reset(static_cast<GDBusConnection*>(g_object_ref(bus)));
}

void InvalidationSender::setDelay(int delay) {
    this->delay = delay;
}

void InvalidationSender::invalidate() {
    if (!bus) {
        return;
    }
    if (timeout_id != 0) {
        return;
    }
    if (delay > 0) {
        timeout_id = g_timeout_add_seconds(delay, &InvalidationSender::callback, static_cast<void*>(this));
    } else {
        InvalidationSender::callback(this);
    }
}

int InvalidationSender::callback(void *data) {
    auto invalidator = static_cast<InvalidationSender*>(data);
    GError *error = nullptr;

    if (!g_dbus_connection_emit_signal(
            invalidator->bus.get(), nullptr,
            SCOPES_DBUS_PATH, SCOPES_DBUS_IFACE, SCOPES_INVALIDATE_RESULTS,
            g_variant_new("(s)", "mediascanner-music"), &error)) {
        fprintf(stderr, "Could not invalidate music scope results: %s\n", error->message);
        g_error_free(error);
        error = nullptr;
    }
    if (!g_dbus_connection_emit_signal(
            invalidator->bus.get(), nullptr,
            SCOPES_DBUS_PATH, SCOPES_DBUS_IFACE, SCOPES_INVALIDATE_RESULTS,
            g_variant_new("(s)", "mediascanner-video"), &error)) {
        fprintf(stderr, "Could not invalidate video scope results: %s\n", error->message);
        g_error_free(error);
        error = nullptr;
    }

    invalidator->timeout_id = 0;
    return G_SOURCE_REMOVE;
}

}
