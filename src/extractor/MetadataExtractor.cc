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

#include "MetadataExtractor.hh"
#include "DetectedFile.hh"
#include "dbus-generated.h"
#include "dbus-marshal.hh"
#include "../mediascanner/MediaFile.hh"
#include "../mediascanner/MediaFileBuilder.hh"
#include "../mediascanner/internal/utils.hh"

#include <glib-object.h>
#include <gio/gio.h>

#include <cstdio>
#include <memory>
#include <string>
#include <stdexcept>

using namespace std;

namespace {
const char BUS_NAME[] = "com.canonical.MediaScanner2.Extractor";
const char BUS_PATH[] = "/com/canonical/MediaScanner2/Extractor";
}

namespace mediascanner {

struct MetadataExtractorPrivate {
    std::unique_ptr<MSExtractor, decltype(&g_object_unref)> proxy;

    MetadataExtractorPrivate() : proxy(nullptr, g_object_unref) {}
};

MetadataExtractor::MetadataExtractor(GDBusConnection *bus) {
    p = new MetadataExtractorPrivate();

    GError *error = nullptr;
    p->proxy.reset(ms_extractor_proxy_new_sync(
            bus, G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START_AT_CONSTRUCTION,
            BUS_NAME, BUS_PATH, nullptr, &error));
    if (not p->proxy) {
        string errortxt(error->message);
        g_error_free(error);
        delete(p);

        string msg = "Failed to create D-Bus proxy: ";
        msg += errortxt;
        throw runtime_error(msg);
    }
}

MetadataExtractor::~MetadataExtractor() {
    delete p;
}

DetectedFile MetadataExtractor::detect(const std::string &filename) {
    std::unique_ptr<GFile, void(*)(void *)> file(
        g_file_new_for_path(filename.c_str()), g_object_unref);
    if (!file) {
        throw runtime_error("Could not create file object");
    }

    GError *error = nullptr;
    std::unique_ptr<GFileInfo, void(*)(void *)> info(
        g_file_query_info(
            file.get(),
            G_FILE_ATTRIBUTE_STANDARD_FAST_CONTENT_TYPE ","
            G_FILE_ATTRIBUTE_ETAG_VALUE,
            G_FILE_QUERY_INFO_NONE, /* cancellable */ nullptr, &error),
        g_object_unref);
    if (!info) {
        string errortxt(error->message);
        g_error_free(error);

        string msg("Query of file info for ");
        msg += filename;
        msg += " failed: ";
        msg += errortxt;
        throw runtime_error(msg);
    }

    string etag(g_file_info_get_etag(info.get()));
    string content_type(g_file_info_get_attribute_string(
        info.get(), G_FILE_ATTRIBUTE_STANDARD_FAST_CONTENT_TYPE));
    if (content_type.empty()) {
        throw runtime_error("Could not determine content type.");
    }

    MediaType type;
    if (content_type.find("audio/") == 0) {
        type = AudioMedia;
    } else if (content_type.find("video/") == 0) {
        type = VideoMedia;
    } else if (content_type.find("image/") == 0) {
        type = ImageMedia;
    } else {
        throw runtime_error(string("File ") + filename + " is not audio or video");
    }
    return DetectedFile(filename, etag, content_type, type);
}

MediaFile MetadataExtractor::extract(const DetectedFile &d) {
    printf("Extracting metadata from %s.\n", d.filename.c_str());

    GError *error = nullptr;
    GVariant *res = nullptr;
    if (!ms_extractor_call_extract_metadata_sync(
            p->proxy.get(), d.filename.c_str(), d.etag.c_str(),
            d.content_type.c_str(), d.type, &res, nullptr, &error)) {
        string errortxt(error->message);
        g_error_free(error);
        delete(p);

        string msg = "Failed to create D-Bus proxy: ";
        msg += errortxt;
        throw runtime_error(msg);
    }
    // Place variant in a unique_ptr so it is guaranteed to be unrefed.
    std::unique_ptr<GVariant,decltype(&g_variant_unref)> result(
        res, g_variant_unref);
    return media_from_variant(result.get());
}

MediaFile MetadataExtractor::fallback_extract(const DetectedFile &d) {
    return MediaFileBuilder(d.filename).setType(d.type);
}

}
