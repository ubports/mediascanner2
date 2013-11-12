/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * Authors:
 *    Jussi Pakkanen <jussi.pakkanen@canonical.com>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of version 3 of the GNU General Public License as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "MediaFile.hh"

using namespace std;

MediaFile::MediaFile(std::string filename, std::string title, std::string date, std::string author, std::string album, std::string album_artist,
        int track_number, int duration, MediaType type) :
    filename(filename), title(title), date(date), author(author), album(album), album_artist(album_artist), track_number(track_number), duration(duration), type(type) {

}

const std::string& MediaFile::getFileName() const noexcept {
    return filename;
}
const std::string& MediaFile::getTitle() const noexcept {
    return title;
}

const std::string& MediaFile::getAuthor() const noexcept {
    return author;
}

const std::string& MediaFile::getAlbum() const noexcept {
    return album;
}

const std::string& MediaFile::getAlbumArtist() const noexcept {
    return album_artist;
}

const std::string& MediaFile::getDate() const noexcept {
    return date;
}

int MediaFile::getTrackNumber() const noexcept {
    return track_number;
}

int MediaFile::getDuration() const noexcept {
    return duration;
}

MediaType MediaFile::getType() const noexcept {
    return type;
}

void MediaFile::setTitle(const std::string &title) noexcept {
    this->title = title;
}

void MediaFile::setAuthor(const std::string &author) noexcept {
    this->author = author;
}

void MediaFile::setAlbum(const std::string &album) noexcept {
    this->album = album;
}

void MediaFile::setAlbumArtist(const std::string &album_artist) noexcept {
    this->album_artist = album_artist;
}

void MediaFile::setDate(const std::string &date) noexcept {
    this->date = date;
}

void MediaFile::setTrackNumber(int track_number) noexcept {
    this->track_number = track_number;
}

void MediaFile::setDuration(int duration) noexcept {
    this->duration = duration;
}

void MediaFile::setType(MediaType type) noexcept {
    this->type = type;
}

bool MediaFile::operator==(const MediaFile &other) const {
    return filename == other.filename && title == other.title && author == other.author &&
        album == other.album && album_artist == other.album_artist && date == other.date && track_number == other.track_number && duration == other.duration && type == other.type;
}

bool MediaFile::operator!=(const MediaFile &other) const {
    return !(*this == other);
}
