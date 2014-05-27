/*
 * Copyright (C) 2014 Canonical, Ltd.
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

#include "MediaFilter.hh"

using std::string;

namespace mediascanner {

struct MediaFilter::Private {
    string artist;
    string album;
    string album_artist;
    string genre;

    bool have_artist;
    bool have_album;
    bool have_album_artist;
    bool have_genre;
};

MediaFilter::MediaFilter() : p(new Private) {
}

MediaFilter::MediaFilter(const MediaFilter &other) : MediaFilter() {
    *p = *other.p;
}

MediaFilter::~MediaFilter() {
    delete p;
}

MediaFilter &MediaFilter::operator=(const MediaFilter &other) {
    *p = *other.p;
    return *this;
}

void MediaFilter::setArtist(const std::string &artist) {
    p->artist = artist;
    p->have_artist = true;
}

void MediaFilter::unsetArtist() {
    p->artist = "";
    p->have_artist = false;
}

bool MediaFilter::hasArtist() const {
    return p->have_artist;
}

const std::string &MediaFilter::getArtist() const {
    return p->artist;
}

void MediaFilter::setAlbum(const std::string &album) {
    p->album = album;
    p->have_album = true;
}

void MediaFilter::unsetAlbum() {
    p->album = "";
    p->have_album = false;
}

bool MediaFilter::hasAlbum() const {
    return p->have_album;
}

const std::string &MediaFilter::getAlbum() const {
    return p->album;
}

void MediaFilter::setAlbumArtist(const std::string &album_artist) {
    p->album_artist = album_artist;
    p->have_album_artist = true;
}

void MediaFilter::unsetAlbumArtist() {
    p->album_artist = "";
    p->have_album_artist = false;
}

bool MediaFilter::hasAlbumArtist() const {
    return p->have_album_artist;
}

const std::string &MediaFilter::getAlbumArtist() const {
    return p->album_artist;
}

void MediaFilter::setGenre(const std::string &genre) {
    p->genre = genre;
    p->have_genre = true;
}

void MediaFilter::unsetGenre() {
    p->genre = "";
    p->have_genre = false;
}

bool MediaFilter::hasGenre() const {
    return p->have_genre;
}

const std::string &MediaFilter::getGenre() const {
    return p->genre;
}

}
