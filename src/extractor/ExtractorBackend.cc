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

#include "ExtractorBackend.hh"
#include "DetectedFile.hh"
#include "GStreamerExtractor.hh"
#include "ImageExtractor.hh"
#include "TaglibExtractor.hh"
#include "../mediascanner/MediaFile.hh"
#include "../mediascanner/MediaFileBuilder.hh"

#include <cstdio>
#include <string>

using namespace std;

namespace mediascanner {

struct ExtractorBackendPrivate {
    ExtractorBackendPrivate(int seconds) : gstreamer(seconds) {}

    GStreamerExtractor gstreamer;
    ImageExtractor image;
    TaglibExtractor taglib;
};

ExtractorBackend::ExtractorBackend(int seconds)
    : p(new ExtractorBackendPrivate(seconds)) {
}

ExtractorBackend::~ExtractorBackend() {
    delete p;
}

MediaFile ExtractorBackend::extract(const DetectedFile &d) {
    printf("Extracting metadata from %s.\n", d.filename.c_str());
    MediaFileBuilder mfb(d.filename);
    mfb.setETag(d.etag);
    mfb.setContentType(d.content_type);
    mfb.setModificationTime(d.mtime);
    mfb.setType(d.type);

    switch (d.type) {
    case ImageMedia:
        p->image.extract(d, mfb);
        break;
    case AudioMedia:
        if (!p->taglib.extract(d, mfb)) {
            p->gstreamer.extract(d, mfb);
        }
        break;
    default:
        p->gstreamer.extract(d, mfb);
        break;
    }

    return mfb;
}

}
