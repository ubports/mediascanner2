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
    MediaType type;

    MediaFilePrivate() :
        filename(""), content_type(""), etag(""), title(""), date(""),
        author(""), album(""), album_artist(""), genre(""),
        disc_number(0), track_number(0), duration(0), type(UnknownMedia) {}
    MediaFilePrivate(const std::string &filename) :
        filename(filename), content_type(""), etag(""), title(""), date(""),
        author(""), album(""), album_artist(""), genre(""),
        disc_number(0), track_number(0), duration(0), type(UnknownMedia) {}

    MediaFilePrivate(const MediaFilePrivate &other) {
        *this = other;
    }
};

}

#endif
