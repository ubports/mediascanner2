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
#include "scannercore.hh"

namespace mediascanner {

class Filter final {
public:
    Filter();
    Filter(const Filter &other);
    Filter(Filter &&other);
    ~Filter();

    Filter &operator=(const Filter &other);
    Filter &operator=(Filter &&other);
    bool operator==(const Filter &other) const;
    bool operator!=(const Filter &other) const;

    void clear();

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

    void setOffset(int offset);
    int getOffset() const;
    void setLimit(int limit);
    int getLimit() const;

    void setOrder(MediaOrder order);
    MediaOrder getOrder() const;
    void setReverse(bool reverse);
    bool getReverse() const;

private:
    struct Private;
    Private *p;
};

}

#endif
