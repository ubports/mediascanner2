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

#ifndef MEDIAFILE_HH
#define MEDIAFILE_HH

#include"scannercore.hh"
#include<string>

namespace mediascanner {

class MediaFileBuilder;
struct MediaFilePrivate;

class MediaFile final {
    friend class MediaFileBuilder;
public:

    MediaFile();
    MediaFile(const MediaFile &other);
    MediaFile(MediaFile &&other);
    MediaFile(const MediaFileBuilder &builder);
    MediaFile(MediaFileBuilder &&builder);
    ~MediaFile();

    const std::string& getFileName() const noexcept;
    const std::string& getContentType() const noexcept;
    const std::string& getETag() const noexcept;
    const std::string& getTitle() const noexcept;
    const std::string& getAuthor() const noexcept;
    const std::string& getAlbum() const noexcept;
    const std::string& getAlbumArtist() const noexcept;
    const std::string& getDate() const noexcept;
    const std::string& getGenre() const noexcept;
    std::string getUri() const;
    std::string getArtUri() const;

    int getDiscNumber() const noexcept;
    int getTrackNumber() const noexcept;
    int getDuration() const noexcept;
    int getWidth() const noexcept;
    int getHeight() const noexcept;
    double getLatitude() const noexcept;
    double getLongitude() const noexcept;
    bool getHasThumbnail() const noexcept;

    MediaType getType() const noexcept;
    bool operator==(const MediaFile &other) const;
    bool operator!=(const MediaFile &other) const;
    MediaFile &operator=(const MediaFile &other);
    MediaFile &operator=(MediaFile &&other);

    // There are no setters. MediaFiles are immutable.
    // For piecewise construction use MediaFileBuilder.

private:
    MediaFilePrivate *p;
};

}

#endif

