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

#include"MediaFileBuilder.hh"
#include"MediaFile.hh"
#include"internal/MediaFilePrivate.hh"

namespace mediascanner {

MediaFileBuilder::MediaFileBuilder(const std::string &fname) :
    p(new MediaFilePrivate(fname)) {
}

MediaFileBuilder::MediaFileBuilder(const MediaFile &mf) :
    p(new MediaFilePrivate(*mf.p)) {
}

MediaFileBuilder::~MediaFileBuilder() {
    delete p;
}

MediaFile MediaFileBuilder::build() const {
    return MediaFile(*this);
}

MediaFileBuilder &MediaFileBuilder::setType(MediaType t) {
    p->type = t;
    return *this;
}

MediaFileBuilder &MediaFileBuilder::setETag(const std::string &e) {
    p->etag = e;
    return *this;
}

MediaFileBuilder &MediaFileBuilder::setContentType(const std::string &c) {
    p->content_type = c;
    return *this;
}

MediaFileBuilder &MediaFileBuilder::setTitle(const std::string &t) {
    p->title = t;
    return *this;
}

MediaFileBuilder &MediaFileBuilder::setDate(const std::string &d) {
    p->date = d;
    return *this;
}

MediaFileBuilder &MediaFileBuilder::setAuthor(const std::string &a) {
    p->author = a;
    return *this;
}

MediaFileBuilder &MediaFileBuilder::setAlbum(const std::string &a) {
    p->album = a;
    return *this;
}

MediaFileBuilder &MediaFileBuilder::setAlbumArtist(const std::string &a) {
    p->album_artist = a;
    return *this;
}

MediaFileBuilder &MediaFileBuilder::setGenre(const std::string &g) {
    p->genre = g;
    return *this;
}

MediaFileBuilder &MediaFileBuilder::setDiscNumber(int n) {
    p->disc_number = n;
    return *this;
}

MediaFileBuilder &MediaFileBuilder::setTrackNumber(int n) {
    p->track_number = n;
    return *this;
}

MediaFileBuilder &MediaFileBuilder::setDuration(int n) {
    p->duration = n;
    return *this;
}

MediaFileBuilder &MediaFileBuilder::setWidth(int w) {
    p->width = w;
    return *this;
}

MediaFileBuilder &MediaFileBuilder::setHeight(int h) {
    p->height = h;
    return *this;
}

MediaFileBuilder &MediaFileBuilder::setLatitude(double l) {
    p->latitude = l;
    return *this;
}

MediaFileBuilder &MediaFileBuilder::setLongitude(double l) {
    p->longitude = l;
    return *this;
}

MediaFileBuilder & MediaFileBuilder::setHasThumbnail(bool t) {
    p->has_thumbnail = t;
    return *this;
}

}
