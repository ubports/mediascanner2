/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * Authors:
 *    James Henstridge <james.henstridge@canonical.com>
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

#ifndef ALBUM_HH
#define ALBUM_HH

#include <string>

namespace mediascanner {

class Album final {
public:

    Album(const std::string &title, const std::string &artist);
    Album() = delete;

    const std::string& getTitle() const noexcept;
    const std::string& getArtist() const noexcept;
    bool operator==(const Album &other) const;
    bool operator!=(const Album &other) const;

    void setTitle(const std::string& title) noexcept;
    void setArtist(const std::string& artist) noexcept;

private:
    std::string title;
    std::string artist;
};

}

#endif
