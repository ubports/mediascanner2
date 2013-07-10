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

#include<cstdio>
#include<map>
#include<string>
#include<stdexcept>
#include<sys/inotify.h>
#include<dirent.h>
#include<sys/stat.h>
#include<unistd.h>

using namespace std;

class SubtreeWatcher {
private:
    int inotifyid;
    map<int, string> dirmap;

public:
    SubtreeWatcher();
    ~SubtreeWatcher();

    void addDir(const string &path);
};

SubtreeWatcher::SubtreeWatcher() {
    inotifyid = inotify_init();
    if(inotifyid == -1)
        throw runtime_error("Could not init inotify.");
}

SubtreeWatcher::~SubtreeWatcher() {
    for(auto &i : dirmap) {
        inotify_rm_watch(inotifyid, i.first);
    }
    close(inotifyid);
}

void SubtreeWatcher::addDir(const string &root) {
    if(root[0] != '/')
        throw runtime_error("Path must be absolute.");
    DIR* dir = opendir(root.c_str());
    printf("In subdir %s\n", root.c_str());
    if(!dir) {
        return;
    }
    int wd = inotify_add_watch(inotifyid, root.c_str(),
            IN_IGNORED | IN_CREATE | IN_DELETE_SELF | IN_DELETE);
    if(wd == -1) {
        throw runtime_error("Could not create inotify watch object.");
    }
    dirmap[wd] = root;
    struct dirent* curloc;
    while( (curloc = readdir(dir)) ) {
        struct stat statbuf;
        string fname = curloc->d_name;
        if(fname == "." || fname == "..")
            continue;
        string fullpath = root + "/" + fname;
        stat(fullpath.c_str(), &statbuf);
        if(S_ISDIR(statbuf.st_mode)) {
            addDir(fullpath);
        }
    }
}


int main(int argc, char **argv) {
    SubtreeWatcher sw;
    if(argc != 2) {
        printf("%s <subdir to watch>\n", argv[0]);
        return 1;
    }
    sw.addDir(argv[1]);
    return 0;
}
