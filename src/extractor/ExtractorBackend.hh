/*
 * Copyright (C) 2013-2014 Canonical, Ltd.
 *
 * Authors:
 *    Jussi Pakkanen <jussi.pakkanen@canonical.com>
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

#ifndef EXTRACTOR_EXTRACTORBACKEND_H
#define EXTRACTOR_EXTRACTORBACKEND_H

#include <string>

namespace mediascanner {

class MediaFile;
struct DetectedFile;
struct ExtractorBackendPrivate;

class ExtractorBackend final {
public:
    explicit ExtractorBackend(int seconds=25);
    ~ExtractorBackend();
    ExtractorBackend(const ExtractorBackend&) = delete;
    ExtractorBackend& operator=(ExtractorBackend &o) = delete;

    MediaFile extract(const DetectedFile &d);

private:
    ExtractorBackendPrivate *p;
};

}

#endif
