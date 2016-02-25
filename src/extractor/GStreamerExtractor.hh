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

#ifndef EXTRACTOR_GSTREAMEREXTRACTOR_H
#define EXTRACTOR_GSTREAMEREXTRACTOR_H

#include <memory>

typedef struct _GstDiscoverer GstDiscoverer;

namespace mediascanner {

class MediaFileBuilder;
struct DetectedFile;

class GStreamerExtractor final {
public:
    explicit GStreamerExtractor(int seconds);
    ~GStreamerExtractor();
    GStreamerExtractor(const GStreamerExtractor&) = delete;
    GStreamerExtractor& operator=(GStreamerExtractor &o) = delete;

    void extract(const DetectedFile &d, MediaFileBuilder &builder);

private:
    std::unique_ptr<GstDiscoverer, void(*)(void*)> discoverer;
};

}

#endif
