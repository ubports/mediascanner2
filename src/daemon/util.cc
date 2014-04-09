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

#include "util.h"

#include<sys/stat.h>
#include<cstring>

using namespace std;

static bool dir_exists(const string &path) {
    struct stat statbuf;
    if(stat(path.c_str(), &statbuf) < 0) {
        if(errno != ENOENT) {
            printf("Error while trying to determine state of dir %s: %s\n", path.c_str(), strerror(errno));
        }
        return false;
    }
    return S_ISDIR(statbuf.st_mode) ;
}

bool is_rootlike(const string &path) {
    string s1 = path + "/usr";
    string s2 = path + "/var";
    string s3 = path + "/bin";
    string s4 = path + "/Program Files";
    return (dir_exists(s1) && dir_exists(s2) && dir_exists(s3)) || dir_exists(s4);
}

bool is_optical_disc(const string &path) {
    string dvd1 = path + "/AUDIO_TS";
    string dvd2 = path + "/VIDEO_TS";
    string bluray = path + "/BDMV";
    return (dir_exists(dvd1) && dir_exists(dvd2)) || dir_exists(bluray);
}
