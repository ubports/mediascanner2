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

    static const int BUFSIZE=4096;

public:
    SubtreeWatcher();
    ~SubtreeWatcher();

    void addDir(const string &path);
    void run();
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
    printf("Watching subdirectory %s\n", root.c_str());
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


void SubtreeWatcher::run() {
    char buf[BUFSIZE];
    while(true) {
        ssize_t num_read;
        num_read = read(inotifyid, buf, BUFSIZE);
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
            string directory = dirmap[event->wd];
            string filename(event->name);
            string abspath = directory + '/' + filename;
            bool is_dir = false;
            bool is_file = false;
            struct stat statbuf;
            stat(abspath.c_str(), &statbuf);
            if(S_ISDIR(statbuf.st_mode))
                is_dir = true;
            if(S_ISREG(statbuf.st_mode))
                is_file = true;
            if(event->mask & IN_CREATE) {
                if(is_dir)
                    printf("New directory was created: %s.\n", abspath.c_str());
                if(is_file)
                    printf("New file was created: %s.\n", abspath.c_str());
            } else if(event->mask & IN_DELETE) {
                if(is_dir)
                    printf("Subdirectory was deleted: %s.\n", abspath.c_str());
                if(is_file)
                    printf("File was deleted: %s\n", abspath.c_str());
            } else {
                printf("Unknown event.\n");
            }
            p += sizeof(struct inotify_event) + event->len;
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
    sw.run();
    return 0;
}
