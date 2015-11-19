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

std::string detect_albumart(const std::string &directory) {
    static const std::array<const char*, 2> art_basenames = {
        "folder",
        "cover",
    };
    static const std::array<const char*, 3> art_extensions = {
        "jpeg",
        "jpg",
        "png",
    };
    for (const auto &base : art_basenames) {
        for (const auto &ext : art_extensions) {
            std::string filename = directory + "/" + base + "." + ext;
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


std::string FolderArtCache::get_folder_art(const std::string &directory) {
    struct stat s;
    if (lstat(directory.c_str(), &s) < 0) {
        std::string message("Could not stat directory: ");
        message += strerror(errno);
        throw std::runtime_error(message);
    }
    if (!S_ISDIR(s.st_mode)) {
        throw std::runtime_error(directory + " is not a directory");
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

    if (info.dir_mtime != s.st_mtime) {
        info.art = detect_albumart(directory);
        info.dir_mtime = s.st_mtime;
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

}
