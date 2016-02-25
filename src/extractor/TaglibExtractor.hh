/*
 * Copyright (C) 2016 Canonical, Ltd.
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

#ifndef EXTRACTOR_TAGLIBEXTRACTOR_H
#define EXTRACTOR_TAGLIBEXTRACTOR_H

#include <string>

namespace mediascanner {

class MediaFileBuilder;
struct DetectedFile;

class TaglibExtractor final {
public:
    TaglibExtractor() = default;
    ~TaglibExtractor() = default;
    TaglibExtractor(const TaglibExtractor&) = delete;
    TaglibExtractor& operator=(TaglibExtractor &o) = delete;

    bool extract(const DetectedFile &d, MediaFileBuilder &builder);
};

}

#endif
