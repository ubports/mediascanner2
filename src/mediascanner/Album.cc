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
#include "internal/FolderArtCache.hh"
#include "internal/utils.hh"

using namespace std;

namespace mediascanner {

struct Album::Private {
    string title;
    string artist;
    string date;
    string genre;
    string filename;
    bool has_thumbnail;
    int artist_count;

    Private() {}
    Private(const string &title, const string &artist,
            const string &date, const string &genre,
            const string &filename, bool has_thumbnail,
            int artist_count)
        : title(title), artist(artist), date(date), genre(genre),
          filename(filename), has_thumbnail(has_thumbnail),
          artist_count(artist_count) {}
    Private(const Private &other) {
        *this = other;
    }
};

Album::Album() : p(new Private){
}

Album::Album(const std::string &title, const std::string &artist)
    : Album(title, artist, "", "", "", false, 1) {
}

Album::Album(const std::string &title, const std::string &artist,
             const std::string &date, const std::string &genre,
             const std::string &filename)
    : Album(title, artist, date, genre, filename, !filename.empty(), 1) {
}

Album::Album(const std::string &title, const std::string &artist,
             const std::string &date, const std::string &genre,
             const std::string &filename, bool has_thumbnail, int artist_count)
    : p(new Private(title, artist, date, genre, filename, has_thumbnail, artist_count)) {
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

const std::string& Album::getGenre() const noexcept {
    return p->genre;
}

const std::string& Album::getArtFile() const noexcept {
    return p->filename;
}

bool Album::getHasThumbnail() const noexcept {
    return p->has_thumbnail;
}

int Album::getArtistCount() const noexcept {
    return p->artist_count;
}

std::string Album::getArtUri() const {
    if (p->has_thumbnail) {
        return make_thumbnail_uri(getUri(p->filename));
    } else {
        auto standalone = FolderArtCache::get().get_art_for_file(p->filename);
        if (!standalone.empty()) {
            return make_thumbnail_uri(getUri(standalone));
        }
        return make_album_art_uri(p->artist, p->title);
    }
}

bool Album::operator==(const Album &other) const {
    return p->title == other.p->title &&
        p->artist == other.p->artist &&
        p->date == other.p->date &&
        p->genre == other.p->genre &&
        p->filename == other.p->filename &&
        p->has_thumbnail == other.p->has_thumbnail;
}

bool Album::operator!=(const Album &other) const {
    return !(*this == other);
}

}
