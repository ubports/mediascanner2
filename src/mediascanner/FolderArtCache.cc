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

#include <algorithm>
#include <array>
#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <memory>
#include <stdexcept>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

namespace {

const int CACHE_SIZE = 50;

string detect_albumart(string directory) {
    static const array<const char*, 5> art_basenames = {
        "cover",
        "album",
        "albumart",
        ".folder",
        "folder",
    };
    static const array<const char*, 3> art_extensions = {
        "jpeg",
        "jpg",
        "png",
    };
    if (!directory.empty() && directory[directory.size()-1] != '/') {
        directory += "/";
    }
    unique_ptr<DIR, decltype(&closedir)> dir(
        opendir(directory.c_str()), &closedir);
    if (!dir) {
        return "";
    }

    const int dirent_size = sizeof(dirent) + fpathconf(dirfd(dir.get()), _PC_NAME_MAX) + 1;
    unique_ptr<struct dirent, decltype(&free)> entry(
        reinterpret_cast<struct dirent*>(malloc(dirent_size)), &free);

    string detected;
    int best_score = 0;
    struct dirent *de = nullptr;
    while (readdir_r(dir.get(), entry.get(), &de) == 0 && de) {
        const string filename(de->d_name);
        auto dot = filename.rfind('.');
        // Ignore files with no extension
        if (dot == string::npos) {
            continue;
        }
        auto basename = filename.substr(0, dot);
        auto extension = filename.substr(dot+1);

        // Does the file name match one of the required names when
        // converted to lower case?
        transform(basename.begin(), basename.end(),
                  basename.begin(), ::tolower);
        transform(extension.begin(), extension.end(),
                  extension.begin(), ::tolower);
        auto base_pos = find(art_basenames.begin(), art_basenames.end(), basename);
        if (base_pos == art_basenames.end()) {
            continue;
        }
        auto ext_pos = find(art_extensions.begin(), art_extensions.end(), extension);
        if (ext_pos == art_extensions.end()) {
            continue;
        }

        int score = (base_pos - art_basenames.begin()) * art_basenames.size() +
            (ext_pos - art_extensions.begin());
        if (detected.empty() || score < best_score) {
            detected = filename;
            best_score = score;
        }
    }
    if (detected.empty()) {
        return detected;
    }
    return directory + detected;
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

string FolderArtCache::get_art_for_directory(const string &directory) {
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
    } catch (const out_of_range &) {
        // Fall back to checking the previous iteration of the cache
        try {
            info = old_cache_.at(directory);
            update = true;
        } catch (const out_of_range &) {
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
            old_cache_ = move(cache_);
            cache_.clear();
        }
    }
    return info.art;
}

string FolderArtCache::get_art_for_file(const string &filename) {
    auto slash = filename.rfind('/');
    if (slash == string::npos) {
        return "";
    }
    auto directory = filename.substr(0, slash + 1);
    return get_art_for_directory(directory);
}

}
