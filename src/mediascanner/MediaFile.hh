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

#ifndef MEDIAFILE_HH
#define MEDIAFILE_HH

#include"scannercore.hh"
#include<string>

class MediaFile final {
public:

    MediaFile(std::string filename) : filename(filename), content_type(""), etag(""), title(""), date(""), author(""),
        album(""), album_artist(""), track_number(0), duration(0), type(UnknownMedia) {}
    MediaFile(std::string filename, std::string content_type, std::string etag, std::string title, std::string date, std::string author, std::string album, std::string album_artist,
              int track_number, int duration, MediaType type);
    MediaFile() = delete;

    const std::string& getFileName() const noexcept;
    const std::string& getContentType() const noexcept;
    const std::string& getETag() const noexcept;
    const std::string& getTitle() const noexcept;
    const std::string& getAuthor() const noexcept;
    const std::string& getAlbum() const noexcept;
    const std::string& getAlbumArtist() const noexcept;
    const std::string& getDate() const noexcept;
    int getTrackNumber() const noexcept;
    int getDuration() const noexcept;
    MediaType getType() const noexcept;
    bool operator==(const MediaFile &other) const;
    bool operator!=(const MediaFile &other) const;

    // There are no setters. MediaFiles are immutable.
    // For piecewise construction use MediaFileBuilder.

private:
    std::string filename;
    std::string content_type;
    std::string etag;
    std::string title;
    std::string date; // ISO date string.  Should this be time since epoch?
    std::string author;
    std::string album;
    std::string album_artist;
    int track_number;
    int duration; // In seconds.
    MediaType type;
};

#endif

