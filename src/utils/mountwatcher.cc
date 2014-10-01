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
#include <glib.h>
#include "../daemon/MountWatcher.hh"

void mount_event(const mediascanner::MountWatcher::Info &info) {
    printf("Filesystem %s\n", info.is_mounted ? "mounted" : "unmounted");
    printf("  Device:      %s\n", info.device.c_str());
    printf("  UUID:        %s\n", info.uuid.c_str());
    printf("  Mount Point: %s\n", info.mount_point.c_str());
    printf("\n");
}

int main(int, char **) {
    mediascanner::MountWatcher watcher(mount_event);
    GMainLoop *main = g_main_loop_new(nullptr, false);
    g_main_loop_run(main);
}
