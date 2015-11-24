/*
 * Copyright (C) 2015 Canonical, Ltd.
 *
 * Authors:
 *    James Henstridge <james.henstridge@canonical.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef FOLDERARTCACHE_HH
#define FOLDERARTCACHE_HH

#include <cstdint>
#include <ctime>
#include <map>
#include <string>

namespace mediascanner {

struct FolderArtInfo {
    std::string art;
    struct timespec dir_mtime {0, 0};
};

class FolderArtCache final {
public:
    FolderArtCache();
    ~FolderArtCache();

    FolderArtCache(const FolderArtCache &other) = delete;
    FolderArtCache& operator=(const FolderArtCache &other) = delete;

    // Get a singleton instance of the cache
    static FolderArtCache& get();

    std::string get_art_for_directory(const std::string &directory);
    std::string get_art_for_file(const std::string &filename);
private:
    std::map<std::string, FolderArtInfo> cache_;
    std::map<std::string, FolderArtInfo> old_cache_;
};

}

#endif
