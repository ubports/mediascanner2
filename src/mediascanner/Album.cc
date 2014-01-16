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

using namespace std;

Album::Album(const std::string &title, const std::string &artist)
    : title(title), artist(artist) {
}

const std::string& Album::getTitle() const noexcept {
    return title;
}

const std::string& Album::getArtist() const noexcept {
    return artist;
}

void Album::setTitle(const std::string &title) noexcept {
    this->title = title;
}

void Album::setArtist(const std::string &artist) noexcept {
    this->artist = artist;
}

bool Album::operator==(const Album &other) const {
    return title == other.title && artist == other.artist;
}

bool Album::operator!=(const Album &other) const {
    return !(*this == other);
}
