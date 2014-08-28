/*
 * Copyright (C) 2013 Canonical, Ltd.
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

#ifndef MEDIAFILEPRIVATE_HH
#define MEDIAFILEPRIVATE_HH

#include <string>

namespace mediascanner {

struct MediaFilePrivate {
    std::string filename;
    std::string content_type;
    std::string etag;
    std::string title;
    std::string date; // ISO date string.  Should this be time since epoch?
    std::string author;
    std::string album;
    std::string album_artist;
    std::string genre;
    int disc_number;
    int track_number;
    int duration; // In seconds.

    int width;
    int height;
    double latitude;  // In degrees, negative for South
    double longitude; // In degrees, negative for West
    bool has_thumbnail;

    MediaType type;

    MediaFilePrivate();
    MediaFilePrivate(const std::string &filename);
    MediaFilePrivate(const MediaFilePrivate &other);

    bool operator==(const MediaFilePrivate &other) const;
    bool operator!=(const MediaFilePrivate &other) const;

    void setFallbackMetadata();
};

}

#endif
