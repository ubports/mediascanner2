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

namespace mediascanner {

class InvalidationSender;
class MediaStore;
class MetadataExtractor;

struct SubtreeWatcherPrivate;

class SubtreeWatcher final {
private:

    SubtreeWatcherPrivate *p;
    bool fileAdded(const std::string &abspath);
    void fileDeleted(const std::string &abspath);
    void dirAdded(const std::string &abspath);
    void dirRemoved(const std::string &abspath);

    bool removeDir(const std::string &abspath);

public:
    SubtreeWatcher(MediaStore &store, MetadataExtractor &extractor, InvalidationSender &invalidator);
    ~SubtreeWatcher();
    SubtreeWatcher(const SubtreeWatcher &o) = delete;
    SubtreeWatcher& operator=(const SubtreeWatcher &o) = delete;

    void addDir(const std::string &path);
    void processEvents();
    int getFd() const;
    int directoryCount() const;
};

}

#endif
