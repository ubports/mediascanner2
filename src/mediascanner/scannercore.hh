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

#ifndef SCANNERCORE_H
#define SCANNERCORE_H

namespace mediascanner {

enum MediaType {
    UnknownMedia,
    AudioMedia,
    VideoMedia,
    ImageMedia,
    AllMedia = 255,
};

enum class MediaOrder {
    Default,
    Rank,
    Title,
    Date,
    Modified,
};

}

#endif
