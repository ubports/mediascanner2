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
#include<memory>

using namespace std;

static void discov_unref(GstDiscoverer *t) {
    g_object_unref(G_OBJECT(t));
}

static void info_unref(GstDiscovererInfo *t) {
    g_object_unref(G_OBJECT(t));
}

struct metadata {
    string author;
    string title;
    string album;
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

    GError *error = NULL;

    // FIXME: possibly share the discoverer between uses.
    unique_ptr<GstDiscoverer, void(*)(GstDiscoverer *)> discoverer(gst_discoverer_new(GST_SECOND * 25, &error),
            discov_unref);
    if (discoverer == NULL) {
        string errortxt(error->message);
        g_error_free(error);

        string msg = "Failed to create discoverer: ";
        msg += errortxt;
        throw runtime_error(msg);
    }

    unique_ptr<GstDiscovererInfo, void(*)(GstDiscovererInfo *)> info(gst_discoverer_discover_uri(discoverer.get(),
            uri.c_str(), &error), info_unref);
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
        gst_tag_list_foreach (tags, extract_tag_info, &md);
    }
    int dur = static_cast<int>(gst_discoverer_info_get_duration(info.get())/GST_SECOND);

    title = md.title;
    author = md.author;
    album = md.album;
    duration = dur;
    return 0;
}
