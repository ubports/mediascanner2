/*
 * Copyright (C) 2013 Canonical, Ltd.
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

#ifndef ALBUM_HH
#define ALBUM_HH

#include <string>

namespace mediascanner {

class Album final {
public:

    Album();
    Album(const std::string &title, const std::string &artist);
    Album(const std::string &title, const std::string &artist,
          const std::string &date, const std::string &genre,
          const std::string &filename);
    Album(const std::string &title, const std::string &artist,
          const std::string &date, const std::string &genre,
          const std::string &filename, bool has_thumbnail, int artist_count);
    Album(const Album &other);
    Album(Album &&other);
    ~Album();

    Album &operator=(const Album &other);
    Album &operator=(Album &&other);

    const std::string& getTitle() const noexcept;
    const std::string& getArtist() const noexcept;
    const std::string& getDate() const noexcept;
    const std::string& getGenre() const noexcept;
    const std::string& getArtFile() const noexcept;
    bool getHasThumbnail() const noexcept;
    int getArtistCount() const noexcept;
    std::string getArtUri() const;
    bool operator==(const Album &other) const;
    bool operator!=(const Album &other) const;

private:
    struct Private;
    Private *p;
};

}

#endif
