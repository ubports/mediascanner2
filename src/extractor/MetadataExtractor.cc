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

#include "MetadataExtractor.hh"
#include "DetectedFile.hh"
#include "dbus-generated.h"
#include "dbus-marshal.hh"
#include "../mediascanner/MediaFile.hh"
#include "../mediascanner/MediaFileBuilder.hh"
#include "../mediascanner/internal/utils.hh"

#include <glib-object.h>
#include <gio/gio.h>

#include <algorithm>
#include <array>
#include <cstdio>
#include <memory>
#include <string>
#include <stdexcept>

using namespace std;

namespace {
const char BUS_NAME[] = "com.canonical.MediaScanner2.Extractor";
const char BUS_PATH[] = "/com/canonical/MediaScanner2/Extractor";

// This list was obtained by grepping /usr/share/mime/audio/.
std::array<const char*, 4> blacklist{{"audio/x-iriver-pla", "audio/x-mpegurl", "audio/x-ms-asx", "audio/x-scpls"}};

void validate_against_blacklist(const std::string &filename, const std::string &content_type) {

    auto result = std::find(blacklist.begin(), blacklist.end(), content_type);
    if(result != blacklist.end()) {
        throw runtime_error("File " + filename + " is of blacklisted type " + content_type + ".");
    }
}

}

namespace mediascanner {

struct MetadataExtractorPrivate {
    std::unique_ptr<GDBusConnection, decltype(&g_object_unref)> bus;
    std::unique_ptr<MSExtractor, decltype(&g_object_unref)> proxy {nullptr, g_object_unref};

    MetadataExtractorPrivate(GDBusConnection *bus);
    void create_proxy();
};

MetadataExtractorPrivate::MetadataExtractorPrivate(GDBusConnection *bus)
    : bus(reinterpret_cast<GDBusConnection*>(g_object_ref(bus)),
          g_object_unref) {
    create_proxy();
}

void MetadataExtractorPrivate::create_proxy() {
    GError *error = nullptr;
    proxy.reset(ms_extractor_proxy_new_sync(
            bus.get(), static_cast<GDBusProxyFlags>(
                G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES |
                G_DBUS_PROXY_FLAGS_DO_NOT_CONNECT_SIGNALS |
                G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START_AT_CONSTRUCTION),
            BUS_NAME, BUS_PATH, nullptr, &error));
    if (not proxy) {
        string errortxt(error->message);
        g_error_free(error);

        string msg = "Failed to create D-Bus proxy: ";
        msg += errortxt;
        throw runtime_error(msg);
    }
}

MetadataExtractor::MetadataExtractor(GDBusConnection *bus) {
    p.reset(new MetadataExtractorPrivate(bus));
}

MetadataExtractor::~MetadataExtractor() = default;

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
            G_FILE_ATTRIBUTE_TIME_MODIFIED ","
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

    uint64_t mtime = g_file_info_get_attribute_uint64(
        info.get(), G_FILE_ATTRIBUTE_TIME_MODIFIED);
    string etag(g_file_info_get_etag(info.get()));
    string content_type(g_file_info_get_attribute_string(
        info.get(), G_FILE_ATTRIBUTE_STANDARD_FAST_CONTENT_TYPE));
    if (content_type.empty()) {
        throw runtime_error("Could not determine content type.");
    }

    validate_against_blacklist(filename, content_type);
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
    return DetectedFile(filename, etag, content_type, mtime, type);
}

MediaFile MetadataExtractor::extract(const DetectedFile &d) {
    fprintf(stderr, "Extracting metadata from %s.\n", d.filename.c_str());

    GError *error = nullptr;
    GVariant *res = nullptr;
    gboolean success = ms_extractor_call_extract_metadata_sync(
            p->proxy.get(), d.filename.c_str(), d.etag.c_str(),
            d.content_type.c_str(), d.mtime, d.type, &res, nullptr, &error);
    // If we get a synthesised "no reply" error, the server probably
    // crashed due to a codec bug.  We retry the extraction once more
    // in case the crash was due to bad cleanup from a previous job.
    if (!success && error->domain == G_DBUS_ERROR &&
        error->code == G_DBUS_ERROR_NO_REPLY) {
        g_error_free(error);
        error = nullptr;
        fprintf(stderr, "No reply from extractor daemon, retrying once.\n");
        // Recreate the proxy, since the old one will have bound to
        // the old instance's unique name.
        p->create_proxy();
        success = ms_extractor_call_extract_metadata_sync(
                p->proxy.get(), d.filename.c_str(), d.etag.c_str(),
                d.content_type.c_str(), d.mtime, d.type, &res, nullptr, &error);
    }
    if (!success) {
        string errortxt(error->message);
        g_error_free(error);

        string msg = "ExtractMetadata D-Bus call failed: ";
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
