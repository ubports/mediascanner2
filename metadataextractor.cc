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

#include<gst/gst.h>
#include<gst/pbutils/gstdiscoverer.h>

#include<cstdio>
#include<string>
#include<stdexcept>
#include<gst/pbutils/pbutils.h>

using namespace std;

struct metadata {
    string author;
    string title;
    string album;
    int duration;
};

static void
extract_tag_info (const GstTagList * list, const gchar * tag, gpointer user_data) {
    struct metadata *md = (struct metadata*) user_data;
    int i, num;
    string tagname(tag);

    num = gst_tag_list_get_tag_size (list, tag);
    for (i = 0; i < num; ++i) {
        const GValue *val;

        val = gst_tag_list_get_value_index (list, tag, i);
        if (G_VALUE_HOLDS_STRING (val)) {
            if(tagname == "artist")
                md->author = g_value_get_string(val);
            if(tagname == "title")
                md->title = g_value_get_string(val);
            if(tagname == "album")
                md->album = g_value_get_string(val);
        }
    }
}

int getMetadata(const std::string &filename, std::string &title, std::string &author,
        std::string &album, int &duration) {
    struct metadata md;
    // FIXME: Need to do quoting. Files with %'s in their names seem to confuse gstreamer.
    string uri = "file://" + filename;

    GstDiscoverer *discoverer;
    GstDiscovererInfo *info;
    GError *error = NULL;

    // FIXME: we should share the discoverer between uses.
    discoverer = gst_discoverer_new(GST_SECOND * 25, &error);
    if (discoverer == NULL) {
        string errortxt(error->message);
        g_error_free(error);

        string msg = "Failed to create discoverer: ";
        msg += errortxt;
        throw runtime_error(msg);
    }

    info = gst_discoverer_discover_uri(discoverer, uri.c_str(), &error);
    if (info == NULL) {
        string errortxt(error->message);
        g_error_free(error);
        g_object_unref(discoverer);

        string msg = "Discovery of file ";
        msg += filename;
        msg += " failed: ";
        msg += errortxt;
        throw runtime_error(msg);
    }

    if (gst_discoverer_info_get_result(info) != GST_DISCOVERER_OK) {
        g_object_unref(info);
        g_object_unref(discoverer);
        throw runtime_error("Unable to discover file " + filename);
    }

    const GstTagList *tags = gst_discoverer_info_get_tags(info);
    if (tags != NULL) {
        gst_tag_list_foreach (tags, extract_tag_info, &md);
    }
    md.duration = static_cast<int>(gst_discoverer_info_get_duration(info)/GST_SECOND);
    g_object_unref(info);
    g_object_unref(discoverer);

    title = md.title;
    author = md.author;
    album = md.album;
    duration = md.duration;
    return 0;
}
