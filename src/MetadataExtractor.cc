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

#include"MetadataExtractor.hh"
#include "FileTypeDetector.hh"

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

MediaFile MetadataExtractor::extract(const std::string &filename) {
    FileTypeDetector d;
    MediaType media_type = d.detect(filename);
    if (media_type == UnknownMedia) {
        throw runtime_error("Tried to create an invalid media type.");
    }

    // FIXME: Need to do quoting. Files with %'s in their names seem to confuse gstreamer.
    string uri = "file://" + filename;

    GError *error = NULL;
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

    struct metadata md;
    const GstTagList *tags = gst_discoverer_info_get_tags(info.get());
    if (tags != NULL) {
        gst_tag_list_foreach (tags, extract_tag_info, &md);
    }
    int duration = static_cast<int>(
        gst_discoverer_info_get_duration(info.get())/GST_SECOND);

    return MediaFile(filename, md.title, md.author, md.album,
                     duration, media_type);
}
