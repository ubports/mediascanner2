/*
 * Copyright (C) 2014 Canonical, Ltd.
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

#ifndef MEDIAFILTER_H_
#define MEDIAFILTER_H_

#include <string>

namespace mediascanner {

class MediaFilter final {
public:
    MediaFilter();
    MediaFilter(const MediaFilter &other);
    ~MediaFilter();

    MediaFilter &operator=(const MediaFilter &other);

    void setArtist(const std::string &artist);
    void unsetArtist();
    bool hasArtist() const;
    const std::string &getArtist() const;

    void setAlbum(const std::string &album);
    void unsetAlbum();
    bool hasAlbum() const;
    const std::string &getAlbum() const;

    void setAlbumArtist(const std::string &album_artist);
    void unsetAlbumArtist();
    bool hasAlbumArtist() const;
    const std::string &getAlbumArtist() const;

    void setGenre(const std::string &genre);
    void unsetGenre();
    bool hasGenre() const;
    const std::string &getGenre() const;

private:
    struct Private;
    Private *p;
};

}

#endif
