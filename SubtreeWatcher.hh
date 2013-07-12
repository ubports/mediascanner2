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

#ifndef SUBTREEWATCHER_HH_
#define SUBTREEWATCHER_HH_

#include<string>
#include<map>

class MediaStore;

class SubtreeWatcher {
private:
    MediaStore *store; // Hackhackhack, in real code replace with callback object or something.
    int inotifyid;
    // Ideally use boost::bimap or something instead of these two separate objects.
    std::map<int, std::string> wd2str;
    std::map<std::string, int> str2wd;
    bool keep_going;

    static const int BUFSIZE=4096;

    void fileAdded(const std::string &abspath);
    void fileDeleted(const std::string &abspath);
    void dirAdded(const std::string &abspath);
    void dirRemoved(const std::string &abspath);

    bool removeDir(const std::string &abspath);

public:
    SubtreeWatcher(MediaStore *store=nullptr);
    ~SubtreeWatcher();

    void addDir(const std::string &path);
    void run();
};


#endif
