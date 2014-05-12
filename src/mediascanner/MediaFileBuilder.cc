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

MediaFile MediaFileBuilder::build() const {
    return MediaFile(*this);
}

void MediaFileBuilder::setType(MediaType t) {
    p->type = t;
}

void MediaFileBuilder::setETag(const std::string &e) {
    p->etag = e;
}

void MediaFileBuilder::setContentType(const std::string &c) {
    p->content_type = c;
}

void MediaFileBuilder::setTitle(const std::string &t) {
    p->title = t;
}

void MediaFileBuilder::setDate(const std::string &d) {
    p->date = d;
}

void MediaFileBuilder::setAuthor(const std::string &a) {
    p->author = a;
}

void MediaFileBuilder::setAlbum(const std::string &a) {
    p->album = a;
}

void MediaFileBuilder::setAlbumArtist(const std::string &a) {
    p->album_artist = a;
}

void MediaFileBuilder::setGenre(const std::string &g) {
    p->genre = g;
}

void MediaFileBuilder::setDiscNumber(int n) {
    p->disc_number = n;
}

void MediaFileBuilder::setTrackNumber(int n) {
    p->track_number = n;
}

void MediaFileBuilder::setDuration(int n) {
    p->duration = n;
}

}
