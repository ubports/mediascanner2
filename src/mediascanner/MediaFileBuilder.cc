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
#include<stdexcept>

namespace mediascanner {

MediaFileBuilder::MediaFileBuilder(const std::string &fname) {
    filename = fname;
}

MediaFileBuilder::MediaFileBuilder(const MediaFile &mf) :
    type(mf.getType()),
    filename(mf.getFileName()),
    content_type(mf.getContentType()),
    etag(mf.getETag()),
    title(mf.getTitle()),
    date(mf.getDate()),
    author(mf.getAuthor()),
    album(mf.getAlbum()),
    album_artist(mf.getAlbumArtist()),
    genre(mf.getGenre()),
    disc_number(mf.getDiscNumber()),
    track_number(mf.getTrackNumber()),
    duration(mf.getDuration()) {
}

MediaFile MediaFileBuilder::build() const {
    return MediaFile(filename, content_type, etag, title, date, author,
        album, album_artist, genre, disc_number, track_number, duration, type);
}

void MediaFileBuilder::setType(MediaType t) {
    if(type_set)
        throw std::invalid_argument("Tried to set type when it was already set.");
    type = t;
    type_set = true;
}

void MediaFileBuilder::setETag(const std::string &e) {
    if(etag_set)
        throw std::invalid_argument("Tried to set filename when it was already set.");
    etag = e;
    etag_set = true;

}
void MediaFileBuilder::setContentType(const std::string &c) {
    if(content_type_set)
        throw std::invalid_argument("Tried to set filename when it was already set.");
    content_type = c;
    content_type_set = true;

}

void MediaFileBuilder::setTitle(const std::string &t) {
    if(title_set)
        throw std::invalid_argument("Tried to set title when it was already set.");
    title = t;
    title_set = true;
}

void MediaFileBuilder::setDate(const std::string &d) {
    if(date_set)
        throw std::invalid_argument("Tried to set date when it was already set.");
    date = d;
    date_set = true;
}

void MediaFileBuilder::setAuthor(const std::string &a) {
    if(author_set)
        throw std::invalid_argument("Tried to set author when it was already set.");
    author = a;
    author_set = true;
}

void MediaFileBuilder::setAlbum(const std::string &a) {
    if(album_set)
        throw std::invalid_argument("Tried to set album when it was already set.");
    album = a;
    album_set = true;
}

void MediaFileBuilder::setAlbumArtist(const std::string &a) {
    if(album_artist_set)
        throw std::invalid_argument("Tried to set album artist when it was already set.");
    album_artist = a;
    album_artist_set = true;
}

void MediaFileBuilder::setGenre(const std::string &g) {
    if(genre_set)
        throw std::invalid_argument("Tried to set genre when it was already set.");
    genre = g;
    genre_set = true;
}

void MediaFileBuilder::setDiscNumber(int n) {
    if(disc_number_set)
        throw std::invalid_argument("Tried to set disc number when it was already set.");
    disc_number = n;
    disc_number_set = true;
}

void MediaFileBuilder::setTrackNumber(int n) {
    if(track_number_set)
        throw std::invalid_argument("Tried to set track number when it was already set.");
    track_number = n;
    track_number_set = true;
}

void MediaFileBuilder::setDuration(int n) {
    if(duration_set)
        throw std::invalid_argument("Tried to set duration when it was already set.");
    duration = n;
    duration_set = true;
}

}
