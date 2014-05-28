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

#include "Filter.hh"

using std::string;

namespace mediascanner {

struct Filter::Private {
    string artist;
    string album;
    string album_artist;
    string genre;

    bool have_artist;
    bool have_album;
    bool have_album_artist;
    bool have_genre;

    Private() :
        have_artist(false), have_album(false), have_album_artist(false),
        have_genre(false) {
    }
};

Filter::Filter() : p(new Private) {
}

Filter::Filter(const Filter &other) : Filter() {
    *p = *other.p;
}

Filter::~Filter() {
    delete p;
}

bool Filter::operator==(const Filter &other) const {
    return
        p->have_artist == other.p->have_artist &&
        p->have_album == other.p->have_album &&
        p->have_album_artist == other.p->have_album_artist &&
        p->have_genre == other.p->have_genre &&
        p->artist == other.p->artist &&
        p->album == other.p->album &&
        p->album_artist == other.p->album_artist &&
        p->genre == other.p->genre;
}

bool Filter::operator!=(const Filter &other) const {
    return !(*this == other);
}

Filter &Filter::operator=(const Filter &other) {
    *p = *other.p;
    return *this;
}

void Filter::clear() {
    unsetArtist();
    unsetAlbum();
    unsetAlbumArtist();
    unsetGenre();
}

void Filter::setArtist(const std::string &artist) {
    p->artist = artist;
    p->have_artist = true;
}

void Filter::unsetArtist() {
    p->artist = "";
    p->have_artist = false;
}

bool Filter::hasArtist() const {
    return p->have_artist;
}

const std::string &Filter::getArtist() const {
    return p->artist;
}

void Filter::setAlbum(const std::string &album) {
    p->album = album;
    p->have_album = true;
}

void Filter::unsetAlbum() {
    p->album = "";
    p->have_album = false;
}

bool Filter::hasAlbum() const {
    return p->have_album;
}

const std::string &Filter::getAlbum() const {
    return p->album;
}

void Filter::setAlbumArtist(const std::string &album_artist) {
    p->album_artist = album_artist;
    p->have_album_artist = true;
}

void Filter::unsetAlbumArtist() {
    p->album_artist = "";
    p->have_album_artist = false;
}

bool Filter::hasAlbumArtist() const {
    return p->have_album_artist;
}

const std::string &Filter::getAlbumArtist() const {
    return p->album_artist;
}

void Filter::setGenre(const std::string &genre) {
    p->genre = genre;
    p->have_genre = true;
}

void Filter::unsetGenre() {
    p->genre = "";
    p->have_genre = false;
}

bool Filter::hasGenre() const {
    return p->have_genre;
}

const std::string &Filter::getGenre() const {
    return p->genre;
}

}
