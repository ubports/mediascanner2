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

#include "GStreamerExtractor.hh"
#include "DetectedFile.hh"
#include "../mediascanner/MediaFile.hh"
#include "../mediascanner/MediaFileBuilder.hh"
#include "../mediascanner/internal/utils.hh"

#include <glib-object.h>
#include <gio/gio.h>
#include <gst/gst.h>
#include <gst/pbutils/pbutils.h>

#include <memory>
#include <string>
#include <stdexcept>

using namespace std;
using mediascanner::MediaFileBuilder;

namespace {

void extract_tag_info (const GstTagList *list, const gchar *tag, void *user_data) {
    MediaFileBuilder *mfb = (MediaFileBuilder *) user_data;
    int i, num;
    string tagname(tag);

    if(tagname == GST_TAG_IMAGE || tagname == GST_TAG_PREVIEW_IMAGE) {
        mfb->setHasThumbnail(true);
        return;
    }
    num = gst_tag_list_get_tag_size (list, tag);
    for (i = 0; i < num; ++i) {
        const GValue *val;

        val = gst_tag_list_get_value_index (list, tag, i);
        if (G_VALUE_HOLDS_STRING(val)) {
            const char *value = g_value_get_string(val);
            if (!value) {
                continue;
            }
            if (tagname == GST_TAG_ARTIST)
                mfb->setAuthor(value);
            else if (tagname == GST_TAG_TITLE)
                mfb->setTitle(value);
            else if (tagname == GST_TAG_ALBUM)
                mfb->setAlbum(value);
            else if (tagname == GST_TAG_ALBUM_ARTIST)
                mfb->setAlbumArtist(value);
            else if (tagname == GST_TAG_GENRE)
                mfb->setGenre(value);
        } else if (G_VALUE_HOLDS(val, GST_TYPE_DATE_TIME)) {
            if (tagname == GST_TAG_DATE_TIME) {
                GstDateTime *dt = static_cast<GstDateTime*>(g_value_get_boxed(val));
                if (!dt) {
                    continue;
                }
                char *dt_string = gst_date_time_to_iso8601_string(dt);
                mfb->setDate(dt_string);
                g_free(dt_string);
            }
        } else if (G_VALUE_HOLDS(val, G_TYPE_DATE)) {
            if (tagname == GST_TAG_DATE) {
                GDate *dt = static_cast<GDate*>(g_value_get_boxed(val));
                if (!dt) {
                    continue;
                }
                char buf[100];
                if (g_date_strftime(buf, sizeof(buf), "%Y-%m-%d", dt) != 0) {
                    mfb->setDate(buf);
                }
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

}

namespace mediascanner {

GStreamerExtractor::GStreamerExtractor(int seconds)
    : discoverer(nullptr, g_object_unref) {
    GError *error = nullptr;

    discoverer.reset(gst_discoverer_new(GST_SECOND * seconds, &error));
    if (not discoverer) {
        string errortxt(error->message);
        g_error_free(error);

        string msg = "Failed to create discoverer: ";
        msg += errortxt;
        throw runtime_error(msg);
    }
    if (error) {
        // Sometimes this is filled in even though no error happens.
        g_error_free(error);
    }
}

GStreamerExtractor::~GStreamerExtractor() = default;

void GStreamerExtractor::extract(const DetectedFile &d, MediaFileBuilder &mfb) {
    string uri = getUri(d.filename);

    GError *error = nullptr;
    unique_ptr<GstDiscovererInfo, void(*)(void *)> info(
        gst_discoverer_discover_uri(discoverer.get(), uri.c_str(), &error),
        g_object_unref);
    if (info.get() == nullptr) {
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
    if (tags != nullptr) {
        gst_tag_list_foreach(tags, extract_tag_info, &mfb);
    }
    mfb.setDuration(static_cast<int>(
        gst_discoverer_info_get_duration(info.get())/GST_SECOND));

    /* Check for video specific information */
    unique_ptr<GList, void(*)(GList*)> streams(
        gst_discoverer_info_get_stream_list(info.get()),
        gst_discoverer_stream_info_list_free);
    for (const GList *l = streams.get(); l != nullptr; l = l->next) {
        auto stream_info = static_cast<GstDiscovererStreamInfo*>(l->data);

        if (GST_IS_DISCOVERER_VIDEO_INFO(stream_info)) {
            mfb.setWidth(gst_discoverer_video_info_get_width(
                             GST_DISCOVERER_VIDEO_INFO(stream_info)));
            mfb.setHeight(gst_discoverer_video_info_get_height(
                              GST_DISCOVERER_VIDEO_INFO(stream_info)));
            break;
        }
    }
}

}
