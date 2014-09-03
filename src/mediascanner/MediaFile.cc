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
#include "MediaFileBuilder.hh"
#include "internal/MediaFilePrivate.hh"
#include "internal/utils.hh"
#include <stdexcept>

using namespace std;

namespace mediascanner {

MediaFile::MediaFile() : p(new MediaFilePrivate) {
}

MediaFile::MediaFile(const MediaFile &other) :
    p(new MediaFilePrivate(*other.p)) {
}

MediaFile::MediaFile(MediaFile &&other) : p(nullptr) {
    *this = std::move(other);
}

MediaFile::MediaFile(const MediaFileBuilder &builder) {
    if(!builder.p) {
        throw logic_error("Tried to construct a Mediafile with an empty MediaFileBuilder.");
    }
    p = new MediaFilePrivate(*builder.p);
    p->setFallbackMetadata();
}

MediaFile::MediaFile(MediaFileBuilder &&builder) {
    if(!builder.p) {
        throw logic_error("Tried to construct a Mediafile with an empty MediaFileBuilder.");
    }
    p = builder.p;
    builder.p = nullptr;
    p->setFallbackMetadata();
}

MediaFile::~MediaFile() {
    delete p;
}

MediaFile &MediaFile::operator=(const MediaFile &other) {
    *p = *other.p;
    return *this;
}

MediaFile &MediaFile::operator=(MediaFile &&other) {
    if (this != &other) {
        delete p;
        p = other.p;
        other.p = nullptr;
    }
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

int MediaFile::getWidth() const noexcept {
    return p->width;
}

int MediaFile::getHeight() const noexcept {
    return p->height;
}

double MediaFile::getLatitude() const noexcept {
    return p->latitude;
}

double MediaFile::getLongitude() const noexcept {
    return p->longitude;
}

bool MediaFile::getHasThumbnail() const noexcept {
    return p->has_thumbnail;
}

MediaType MediaFile::getType() const noexcept {
    return p->type;
}

std::string MediaFile::getUri() const {
    return mediascanner::getUri(p->filename);
}

std::string MediaFile::getArtUri() const {
    switch (p->type) {
    case AudioMedia:
        if (p->has_thumbnail) {
            return make_thumbnail_uri(getUri());
        } else {
            return make_album_art_uri(getAuthor(), getAlbum());
        }
    default:
        return make_thumbnail_uri(getUri());
    }
}

bool MediaFile::operator==(const MediaFile &other) const {
    return *p == *other.p;
}

bool MediaFile::operator!=(const MediaFile &other) const {
    return !(*this == other);
}

}
