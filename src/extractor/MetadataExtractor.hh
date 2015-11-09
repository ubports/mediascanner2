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

#ifndef METADATAEXTRACTOR_H
#define METADATAEXTRACTOR_H

#include <cstdint>
#include <memory>
#include <string>
#include "../mediascanner/scannercore.hh"

typedef struct _GDBusConnection GDBusConnection;

namespace mediascanner {

class MediaFile;
struct DetectedFile;
struct MetadataExtractorPrivate;

class MetadataExtractor final {
public:
    explicit MetadataExtractor(GDBusConnection *bus);
    ~MetadataExtractor();
    MetadataExtractor(const MetadataExtractor&) = delete;
    MetadataExtractor& operator=(MetadataExtractor &o) = delete;

    DetectedFile detect(const std::string &filename);
    MediaFile extract(const DetectedFile &d);

    // In case the detected file is know to crash gstreamer,
    // use this to generate fallback data.
    MediaFile fallback_extract(const DetectedFile &d);

private:
    std::unique_ptr<MetadataExtractorPrivate> p;
};

}

#endif
