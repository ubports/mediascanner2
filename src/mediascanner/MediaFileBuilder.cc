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

#include"MediaFileBuilder.hh"
#include"MediaFile.hh"
#include<stdexcept>

MediaFileBuilder::MediaFileBuilder() : type(UnknownMedia) {
}

MediaFile MediaFileBuilder::build() const {
    if(!type_set)
        throw std::invalid_argument("Type is not set in builder.");
    return MediaFile("dummy");
}

void MediaFileBuilder::setType(MediaType t) {
    if(type_set)
        throw std::invalid_argument("Tried to set type when it was already set.");
    type = t;
    type_set = true;
}
