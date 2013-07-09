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

#include<sys/inotify.h>
#include<cstdio>
#include<string>
#include<unistd.h>

using namespace std;

#define BUFSIZE 4096

int main(int /*argc*/, char **/*argv*/) {
    string mount_point = "/media/";
    mount_point += getlogin();
    int ifd;
    int wd;
    char buf[BUFSIZE];

    ifd = inotify_init();
    if(ifd == -1) {
        printf("Inotify init failed.\n");
        return 1;
    }
    wd = inotify_add_watch(ifd, mount_point.c_str(), IN_CREATE | IN_DELETE);
    if(wd == -1) {
        printf("Could not create watch for mount point.\n");
        return 1;
    }
    printf("Watching location %s.\n", mount_point.c_str());
    while(true) {
        ssize_t num_read;
        num_read = read(ifd, buf, BUFSIZE);
        if(num_read == 0) {
            printf("Inotify returned 0.\n");
            return 1;
        }
        if(num_read == -1) {
            printf("Read error.\n");
            return 1;
        }
        for(char *p = buf; p < buf + num_read;) {
            struct inotify_event *event = (struct inotify_event *) p;
            if(event->mask & IN_CREATE) {
                printf("File system was mounted: %s.\n", event->name);
            } else if(event->mask & IN_DELETE) {
                printf("File system was unmounted: %s\n", event->name);
            } else {
                printf("Unknown event.\n");
            }
            p += sizeof(struct inotify_event) + event->len;
        }
    }
    return 0;
}
