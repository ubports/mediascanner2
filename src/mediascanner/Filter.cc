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

    int offset = 0;
    int limit = -1;

    MediaOrder order = MediaOrder::Default;
    bool reverse = false;

    bool have_artist = false;
    bool have_album = false;
    bool have_album_artist = false;
    bool have_genre = false;

    Private() {}
};

Filter::Filter() : p(new Private) {
}

Filter::Filter(const Filter &other) : Filter() {
    *p = *other.p;
}

Filter::Filter(Filter &&other) : p(nullptr) {
    *this = std::move(other);
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
        p->genre == other.p->genre &&
        p->offset == other.p->offset &&
        p->limit == other.p->limit &&
        p->order == other.p->order &&
        p->reverse == other.p->reverse;
}

bool Filter::operator!=(const Filter &other) const {
    return !(*this == other);
}

Filter &Filter::operator=(const Filter &other) {
    *p = *other.p;
    return *this;
}

Filter &Filter::operator=(Filter &&other) {
    if (this != &other) {
        delete p;
        p = other.p;
        other.p = nullptr;
    }
    return *this;
}

void Filter::clear() {
    unsetArtist();
    unsetAlbum();
    unsetAlbumArtist();
    unsetGenre();
    p->offset = 0;
    p->limit = -1;
    p->order = MediaOrder::Default;
    p->reverse = false;
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

void Filter::setOffset(int offset) {
    p->offset = offset;
}

int Filter::getOffset() const {
    return p->offset;
}

void Filter::setLimit(int limit) {
    p->limit = limit;
}

int Filter::getLimit() const {
    return p->limit;
}

void Filter::setOrder(MediaOrder order) {
    p->order = order;
}

MediaOrder Filter::getOrder() const {
    return p->order;
}

void Filter::setReverse(bool reverse) {
    p->reverse = reverse;
}

bool Filter::getReverse() const {
    return p->reverse;
}

}
