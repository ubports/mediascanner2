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

#ifndef SCANNER_HH_
#define SCANNER_HH_

#include<string>
#include<vector>
#include<exception>

#include <mediascanner/scannercore.hh>

namespace mediascanner {

struct DetectedFile;
class MetadataExtractor;

class StopIteration : public std::exception {
};


class Scanner final {
public:
    Scanner(MetadataExtractor *extractor, const std::string &root, const MediaType type);
    ~Scanner();
    Scanner(const Scanner &o) = delete;
    Scanner& operator=(const Scanner &o) = delete;

    DetectedFile next();

private:
    struct Private;
    Private *p;
};

}

#endif
