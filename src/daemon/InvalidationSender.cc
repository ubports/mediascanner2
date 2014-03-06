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

using namespace std;

// timer delay in seconds
const unsigned int DELAY = 1;

InvalidationSender::InvalidationSender() : enabled(true), timeout_id(0) {
}

InvalidationSender::~InvalidationSender() {
    if (timeout_id != 0) {
        g_source_remove(timeout_id);
    }
}

void InvalidationSender::invalidate() {
    if (!enabled) {
        return;
    }
    if (timeout_id != 0) {
        return;
    }
    timeout_id = g_timeout_add_seconds(DELAY, &InvalidationSender::callback, static_cast<void*>(this));
}

int InvalidationSender::callback(void *data) {
    auto invalidator = static_cast<InvalidationSender*>(data);

    string invocation("dbus-send /com/canonical/unity/scopes ");
    invocation += "com.canonical.unity.scopes.InvalidateResults string:";
    const string m_invoc = invocation + "mediascanner-music";
    const string v_invoc = invocation + "mediascanner-video";
    if(system(m_invoc.c_str()) != 0) {
        fprintf(stderr, "Could not invalidate music scope results.\n");
    }
    if(system(v_invoc.c_str()) != 0) {
        fprintf(stderr, "Could not invalidate video scope results.\n");
    }

    invalidator->timeout_id = 0;
    return FALSE;
}

void InvalidationSender::disable() {
    enabled = false;
}
