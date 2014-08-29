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

#ifndef SCAN_UTILS_H
#define SCAN_UTILS_H

#include<string>

namespace mediascanner {

std::string sqlQuote(const std::string &input);
std::string filenameToTitle(const std::string &filename);
std::string getUri(const std::string &filename);
bool is_rootlike(const std::string &path);
bool is_optical_disc(const std::string &path);
bool has_scanblock(const std::string &path);

std::string make_album_art_uri(const std::string &artist, const std::string &album);
std::string make_thumbnail_uri(const std::string &uri);

}

#endif
