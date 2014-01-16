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
#include "utils.hh"

using namespace std;

namespace mediascanner {

MediaFile::MediaFile(std::string filename, std::string content_type, std::string etag, std::string title, std::string date, std::string author, std::string album, std::string album_artist,
        int track_number, int duration, MediaType type) :
    filename(filename), content_type(content_type), etag(etag), title(title), date(date), author(author), album(album), album_artist(album_artist), track_number(track_number), duration(duration), type(type) {

}

const std::string& MediaFile::getFileName() const noexcept {
    return filename;
}

const std::string& MediaFile::getContentType() const noexcept {
    return content_type;
}

const std::string& MediaFile::getETag() const noexcept {
    return etag;
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

std::string MediaFile::getUri() const {
    return mediascanner::getUri(filename);
}

bool MediaFile::operator==(const MediaFile &other) const {
    return
        filename == other.filename &&
        content_type == other.content_type &&
        etag == other.etag &&
        title == other.title &&
        author == other.author &&
        album == other.album &&
        album_artist == other.album_artist &&
        date == other.date &&
        track_number == other.track_number &&
        duration == other.duration &&
        type == other.type;
}

bool MediaFile::operator!=(const MediaFile &other) const {
    return !(*this == other);
}

}
