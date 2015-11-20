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

#include "internal/FolderArtCache.hh"

#include <array>
#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace {

const int CACHE_SIZE = 50;

std::string detect_albumart(std::string directory) {
    static const std::array<const char*, 5> art_basenames = {
        "cover",
        "album",
        "albumart",
        ".folder",
        "folder",
    };
    static const std::array<const char*, 3> art_extensions = {
        "jpeg",
        "jpg",
        "png",
    };
    if (!directory.empty() && directory[directory.size()-1] != '/') {
        directory += "/";
    }
    for (const auto &base : art_basenames) {
        for (const auto &ext : art_extensions) {
            std::string filename = directory + base + "." + ext;
            struct stat s;
            if (stat(filename.c_str(), &s) == 0 && S_ISREG(s.st_mode)) {
                return filename;
            }
        }
    }
    return "";
}

}

namespace mediascanner {

FolderArtCache::FolderArtCache() = default;
FolderArtCache::~FolderArtCache() = default;

// Get a singleton instance of the cache
FolderArtCache& FolderArtCache::get() {
    static FolderArtCache cache;
    return cache;
}

std::string FolderArtCache::get_art_for_directory(const std::string &directory) {
    struct stat s;
    if (lstat(directory.c_str(), &s) < 0) {
        return "";
    }
    if (!S_ISDIR(s.st_mode)) {
        return "";
    }
    FolderArtInfo info;
    bool update = false;
    try {
        info = cache_.at(directory);
    } catch (const std::out_of_range &) {
        // Fall back to checking the previous iteration of the cache
        try {
            info = old_cache_.at(directory);
            update = true;
        } catch (const std::out_of_range &) {
        }
    }

    if (info.dir_mtime.tv_sec != s.st_mtim.tv_sec ||
        info.dir_mtime.tv_nsec != s.st_mtim.tv_nsec) {
        info.art = detect_albumart(directory);
        info.dir_mtime = s.st_mtim;
        update = true;
    }

    if (update) {
        cache_[directory] = info;
        // Start new cache generation if we've exceeded the size.
        if (cache_.size() > CACHE_SIZE) {
            old_cache_ = std::move(cache_);
            cache_.clear();
        }
    }
    return info.art;
}

std::string FolderArtCache::get_art_for_file(const std::string &filename) {
    auto slash = filename.rfind('/');
    if (slash == std::string::npos) {
        return "";
    }
    auto directory = filename.substr(0, slash + 1);
    return get_art_for_directory(directory);
}

}
