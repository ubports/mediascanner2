/*
 * Copyright (C) 2014 Canonical, Ltd.
 *
 * Authors:
 *    James Henstridge <james.henstridge@canonical.com>
 *    Jussi Pakkanen <jussi.pakkanen@canonical.com>
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

#include "scannercore.hh"
#include "internal/MediaFilePrivate.hh"
#include "internal/utils.hh"

namespace mediascanner {

MediaFilePrivate::MediaFilePrivate() :
    disc_number(0), track_number(0), duration(0), width(0), height(0),
    latitude(0.0), longitude(0.0), has_thumbnail(false), type(UnknownMedia) {
}

MediaFilePrivate::MediaFilePrivate(const std::string &filename) :
    filename(filename),
    disc_number(0), track_number(0), duration(0), width(0), height(0),
    latitude(0.0), longitude(0.0), has_thumbnail(false), type(UnknownMedia) {
}

MediaFilePrivate::MediaFilePrivate(const MediaFilePrivate &other) {
    *this = other;
}

bool MediaFilePrivate::operator==(const MediaFilePrivate &other) const {
    return
        filename == other.filename &&
        content_type == other.content_type &&
        etag == other.etag &&
        title == other.title &&
        author == other.author &&
        album == other.album &&
        album_artist == other.album_artist &&
        date == other.date &&
        genre == other.genre &&
        disc_number == other.disc_number &&
        track_number == other.track_number &&
        duration == other.duration &&
        width == other.width &&
        height == other.height &&
        latitude == other.latitude &&
        longitude == other.longitude &&
        has_thumbnail == other.has_thumbnail &&
        type == other.type;
}

bool MediaFilePrivate::operator!=(const MediaFilePrivate &other) const {
    return !(*this == other);
}

void MediaFilePrivate::setFallbackMetadata() {
    if (title.empty()) {
        title = filenameToTitle(filename);
    }
    if (album_artist.empty()) {
        album_artist = author;
    }
}


}
