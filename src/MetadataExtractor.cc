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

#include"MediaFile.hh"
#include"MetadataExtractor.hh"
#include"FileTypeDetector.hh"

#include<gst/gst.h>
#include<gst/pbutils/pbutils.h>

#include<cstdio>
#include<string>
#include<stdexcept>
#include<memory>

using namespace std;

struct MetadataExtractorPrivate {
    std::unique_ptr<GstDiscoverer,void(*)(void *)> discoverer;
    MetadataExtractorPrivate() : discoverer(nullptr, g_object_unref) {};
};

MetadataExtractor::MetadataExtractor(int seconds) {
    p = new MetadataExtractorPrivate();
    GError *error = NULL;

    p->discoverer.reset(gst_discoverer_new(GST_SECOND * seconds, &error));
    if (not p->discoverer) {
        string errortxt(error->message);
        g_error_free(error);

        string msg = "Failed to create discoverer: ";
        msg += errortxt;
        throw runtime_error(msg);
    }
}

MetadataExtractor::~MetadataExtractor() {
    delete p;
}

static void
extract_tag_info (const GstTagList * list, const gchar * tag, gpointer user_data) {
    MediaFile *mf = (MediaFile *) user_data;
    int i, num;
    string tagname(tag);

    num = gst_tag_list_get_tag_size (list, tag);
    for (i = 0; i < num; ++i) {
        const GValue *val;

        val = gst_tag_list_get_value_index (list, tag, i);
        if (G_VALUE_HOLDS_STRING(val)) {
            if (tagname == GST_TAG_ARTIST)
                mf->setAuthor(g_value_get_string(val));
            else if (tagname == GST_TAG_TITLE)
                mf->setTitle(g_value_get_string(val));
            else if (tagname == GST_TAG_ALBUM)
                mf->setAlbum(g_value_get_string(val));
        } else if (G_VALUE_HOLDS(val, GST_TYPE_DATE_TIME)) {
            if (tagname == GST_TAG_DATE_TIME) {
                GstDateTime *dt = static_cast<GstDateTime*>(g_value_get_boxed(val));
                char *dt_string = gst_date_time_to_iso8601_string(dt);
                mf->setDate(dt_string);
                g_free(dt_string);
            }
        } else if (G_VALUE_HOLDS_UINT(val)) {
            if (tagname == GST_TAG_TRACK_NUMBER) {
                mf->setTrackNumber(g_value_get_uint(val));
            }
        }

    }
}

MediaFile MetadataExtractor::extract(const std::string &filename) {
    FileTypeDetector d;
    MediaType media_type = d.detect(filename);
    if (media_type == UnknownMedia) {
        throw runtime_error("Tried to create an invalid media type.");
    }

    MediaFile mf(filename);
    mf.setType(media_type);

    GError *error = nullptr;
    gchar *uristr = gst_filename_to_uri(filename.c_str(), &error);
    if(error) {
        string msg("Could not build URI: ");
        msg += error->message;
        g_error_free(error);
        throw runtime_error(msg);
    }
    string uri(uristr);
    g_free(uristr);

    unique_ptr<GstDiscovererInfo, void(*)(void *)> info(
        gst_discoverer_discover_uri(p->discoverer.get(), uri.c_str(), &error),
        g_object_unref);
    if (info.get() == NULL) {
        string errortxt(error->message);
        g_error_free(error);

        string msg = "Discovery of file ";
        msg += filename;
        msg += " failed: ";
        msg += errortxt;
        throw runtime_error(msg);
    }

    if (gst_discoverer_info_get_result(info.get()) != GST_DISCOVERER_OK) {
        throw runtime_error("Unable to discover file " + filename);
    }

    const GstTagList *tags = gst_discoverer_info_get_tags(info.get());
    if (tags != NULL) {
        gst_tag_list_foreach(tags, extract_tag_info, &mf);
    }
    mf.setDuration(static_cast<int>(
        gst_discoverer_info_get_duration(info.get())/GST_SECOND));

    return mf;
}
