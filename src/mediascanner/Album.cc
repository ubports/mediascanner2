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

#include "Album.hh"
#include "internal/utils.hh"

using namespace std;

namespace mediascanner {

Album::Album() {
}

Album::Album(const std::string &title, const std::string &artist)
    : title(title), artist(artist) {
}

const std::string& Album::getTitle() const noexcept {
    return title;
}

const std::string& Album::getArtist() const noexcept {
    return artist;
}

std::string Album::getArtUri() const {
    return make_album_art_uri(artist, title);
}

bool Album::operator==(const Album &other) const {
    return title == other.title && artist == other.artist;
}

bool Album::operator!=(const Album &other) const {
    return !(*this == other);
}

}
