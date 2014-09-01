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

struct Album::Private {
    string title;
    string artist;
    string date;
    string art_uri;

    Private() {}
    Private(const string &title, const string &artist,
            const string &date, const string &art_uri)
        : title(title), artist(artist), date(date), art_uri(art_uri) {}
    Private(const Private &other) {
        *this = other;
    }
};

Album::Album() : p(new Private){
}

Album::Album(const std::string &title, const std::string &artist,
             const std::string &date, const std::string &art_uri)
    : p(new Private(title, artist, date, art_uri)) {
}

Album::Album(const Album &other) : p(new Private(*other.p)) {
}

Album::Album(Album &&other) : p(nullptr) {
    *this = std::move(other);
}

Album::~Album() {
    delete p;
}

Album &Album::operator=(const Album &other) {
    *p = *other.p;
    return *this;
}

Album &Album::operator=(Album &&other) {
    if (this != &other) {
        delete p;
        p = other.p;
        other.p = nullptr;
    }
    return *this;
}

const std::string& Album::getTitle() const noexcept {
    return p->title;
}

const std::string& Album::getArtist() const noexcept {
    return p->artist;
}

const std::string& Album::getDate() const noexcept {
    return p->date;
}

const std::string& Album::getArtUri() const noexcept {
    return p->art_uri;
}

bool Album::operator==(const Album &other) const {
    return p->title == other.p->title &&
        p->artist == other.p->artist &&
        p->date == other.p->date &&
        p->art_uri == other.p->art_uri;
}

bool Album::operator!=(const Album &other) const {
    return !(*this == other);
}

}
