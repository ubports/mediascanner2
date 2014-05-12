/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * Authors:
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

#include "MediaFile.hh"
#include "internal/MediaFilePrivate.hh"
#include "internal/utils.hh"

using namespace std;

namespace mediascanner {

MediaFile::MediaFile(std::string filename) :
    p(new MediaFilePrivate(filename)) {
}

MediaFile::MediaFile(std::string filename, std::string content_type, std::string etag, std::string title, std::string date, std::string author, std::string album, std::string album_artist, std::string genre,
    int disc_number, int track_number, int duration, MediaType type) :
    p(new MediaFilePrivate(filename, content_type, etag, title, date, author, album, album_artist, genre, disc_number, track_number, duration, type)) {
}

MediaFile::MediaFile(const MediaFile &other) :
    p(new MediaFilePrivate(*other.p)) {
}

MediaFile::~MediaFile() {
    delete p;
}

MediaFile &MediaFile::operator=(const MediaFile &other) {
    *p = *other.p;
    return *this;
}

const std::string& MediaFile::getFileName() const noexcept {
    return p->filename;
}

const std::string& MediaFile::getContentType() const noexcept {
    return p->content_type;
}

const std::string& MediaFile::getETag() const noexcept {
    return p->etag;
}

const std::string& MediaFile::getTitle() const noexcept {
    return p->title;
}

const std::string& MediaFile::getAuthor() const noexcept {
    return p->author;
}

const std::string& MediaFile::getAlbum() const noexcept {
    return p->album;
}

const std::string& MediaFile::getAlbumArtist() const noexcept {
    return p->album_artist;
}

const std::string& MediaFile::getDate() const noexcept {
    return p->date;
}

const std::string& MediaFile::getGenre() const noexcept {
    return p->genre;
}

int MediaFile::getDiscNumber() const noexcept {
    return p->disc_number;
}

int MediaFile::getTrackNumber() const noexcept {
    return p->track_number;
}

int MediaFile::getDuration() const noexcept {
    return p->duration;
}

MediaType MediaFile::getType() const noexcept {
    return p->type;
}

std::string MediaFile::getUri() const {
    return mediascanner::getUri(p->filename);
}

bool MediaFile::operator==(const MediaFile &other) const {
    return
        p->filename == other.p->filename &&
        p->content_type == other.p->content_type &&
        p->etag == other.p->etag &&
        p->title == other.p->title &&
        p->author == other.p->author &&
        p->album == other.p->album &&
        p->album_artist == other.p->album_artist &&
        p->date == other.p->date &&
        p->genre == other.p->genre &&
        p->disc_number == other.p->disc_number &&
        p->track_number == other.p->track_number &&
        p->duration == other.p->duration &&
        p->type == other.p->type;
}

bool MediaFile::operator!=(const MediaFile &other) const {
    return !(*this == other);
}

}
