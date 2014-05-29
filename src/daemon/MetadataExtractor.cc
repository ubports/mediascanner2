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

#include "../mediascanner/MediaFile.hh"
#include "../mediascanner/MediaFileBuilder.hh"
#include "../mediascanner/internal/utils.hh"
#include "MetadataExtractor.hh"

#include <glib-object.h>
#include <gio/gio.h>
#include <gst/gst.h>
#include <gst/pbutils/pbutils.h>

#include<cstdio>
#include<string>
#include<stdexcept>
#include<memory>

using namespace std;

namespace mediascanner {

struct MetadataExtractorPrivate {
    std::unique_ptr<GstDiscoverer, decltype(&g_object_unref)> discoverer;
    MetadataExtractorPrivate() : discoverer(nullptr, g_object_unref) {};
};

MetadataExtractor::MetadataExtractor(int seconds) {
    p = new MetadataExtractorPrivate();
    GError *error = nullptr;

    p->discoverer.reset(gst_discoverer_new(GST_SECOND * seconds, &error));
    if (not p->discoverer) {
        string errortxt(error->message);
        g_error_free(error);
        delete(p);

        string msg = "Failed to create discoverer: ";
        msg += errortxt;
        throw runtime_error(msg);
    }
    if (error) {
        // Sometimes this is filled in even though no error happens.
        g_error_free(error);
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
            G_FILE_QUERY_INFO_NONE, /* cancellable */ NULL, &error),
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
    } else {
        throw runtime_error(string("File ") + filename + " is not audio or video");
    }
    return DetectedFile(filename, etag, content_type, type);
}

static void
extract_tag_info (const GstTagList * list, const gchar * tag, gpointer user_data) {
    MediaFileBuilder *mfb = (MediaFileBuilder *) user_data;
    int i, num;
    string tagname(tag);

    num = gst_tag_list_get_tag_size (list, tag);
    for (i = 0; i < num; ++i) {
        const GValue *val;

        val = gst_tag_list_get_value_index (list, tag, i);
        if (G_VALUE_HOLDS_STRING(val)) {
            if (tagname == GST_TAG_ARTIST)
                mfb->setAuthor(g_value_get_string(val));
            else if (tagname == GST_TAG_TITLE)
                mfb->setTitle(g_value_get_string(val));
            else if (tagname == GST_TAG_ALBUM)
                mfb->setAlbum(g_value_get_string(val));
            else if (tagname == GST_TAG_ALBUM_ARTIST)
                mfb->setAlbumArtist(g_value_get_string(val));
            else if (tagname == GST_TAG_GENRE)
                mfb->setGenre(g_value_get_string(val));
        } else if (G_VALUE_HOLDS(val, GST_TYPE_DATE_TIME)) {
            if (tagname == GST_TAG_DATE_TIME) {
                GstDateTime *dt = static_cast<GstDateTime*>(g_value_get_boxed(val));
                char *dt_string = gst_date_time_to_iso8601_string(dt);
                mfb->setDate(dt_string);
                g_free(dt_string);
            }
        } else if (G_VALUE_HOLDS_UINT(val)) {
            if (tagname == GST_TAG_TRACK_NUMBER) {
                mfb->setTrackNumber(g_value_get_uint(val));
            } else if (tagname == GST_TAG_ALBUM_VOLUME_NUMBER) {
                mfb->setDiscNumber(g_value_get_uint(val));
            }
        }

    }
}

MediaFile MetadataExtractor::extract(const DetectedFile &d) {
    printf("Extracting metadata from %s.\n", d.filename.c_str());
    MediaFileBuilder mfb(d.filename);
    mfb.setETag(d.etag);
    mfb.setContentType(d.content_type);
    mfb.setType(d.type);
    string uri = getUri(d.filename);

    GError *error = nullptr;
    unique_ptr<GstDiscovererInfo, void(*)(void *)> info(
        gst_discoverer_discover_uri(p->discoverer.get(), uri.c_str(), &error),
        g_object_unref);
    if (info.get() == NULL) {
        string errortxt(error->message);
        g_error_free(error);

        string msg = "Discovery of file ";
        msg += d.filename;
        msg += " failed: ";
        msg += errortxt;
        throw runtime_error(msg);
    }
    if (error) {
        // Sometimes this gets filled in even if no error actually occurs.
        g_error_free(error);
        error = nullptr;
    }

    if (gst_discoverer_info_get_result(info.get()) != GST_DISCOVERER_OK) {
        throw runtime_error("Unable to discover file " + d.filename);
    }

    const GstTagList *tags = gst_discoverer_info_get_tags(info.get());
    if (tags != NULL) {
        gst_tag_list_foreach(tags, extract_tag_info, &mfb);
    }
    mfb.setDuration(static_cast<int>(
        gst_discoverer_info_get_duration(info.get())/GST_SECOND));
    return mfb;
}

}
