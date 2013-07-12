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
#include<cstdio>
#include<string>
#include<stdexcept>

using namespace std;

struct metadata {
    string author;
    string title;
    string album;
};

static void
print_one_tag (const GstTagList * list, const gchar * tag, gpointer user_data) {
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
                md->album == g_value_get_string(val);
            /*
              || tagname == "title" || tagname == "album" || tagname == "genre") {
              printf("%s: %s\n", tag, g_value_get_string (val));
          */
        }
    }
}

static void on_new_pad (GstElement * /*dec*/, GstPad * pad, GstElement * fakesink) {
    GstPad *sinkpad;

    sinkpad = gst_element_get_static_pad (fakesink, "sink");
    if (!gst_pad_is_linked (sinkpad)) {
        if (gst_pad_link (pad, sinkpad) != GST_PAD_LINK_OK)
            g_error ("Failed to link pads!");
    }
    gst_object_unref (sinkpad);
}


int getMetadata(const std::string &filename, std::string &title, std::string &author,
        std::string &album) {
    struct metadata md;
    // FIXME: Need to do quoting. Files with %'s in their names seem to confuse gstreamer.
    string uri = "file://" + filename;
    GstElement *pipe, *dec, *sink;
    GstMessage *msg;

    pipe = gst_pipeline_new ("pipeline");

    dec = gst_element_factory_make ("uridecodebin", NULL);
    g_object_set (dec, "uri", uri.c_str(), NULL);
    gst_bin_add (GST_BIN (pipe), dec);

    sink = gst_element_factory_make ("fakesink", NULL);
    gst_bin_add (GST_BIN (pipe), sink);
    g_signal_connect (dec, "pad-added", G_CALLBACK (on_new_pad), sink);

    gst_element_set_state (pipe, GST_STATE_PAUSED);

    while (TRUE) {
      GstTagList *tags = NULL;

      msg = gst_bus_timed_pop_filtered(GST_ELEMENT_BUS (pipe),
          GST_CLOCK_TIME_NONE, (GstMessageType) (GST_MESSAGE_ASYNC_DONE | GST_MESSAGE_TAG | GST_MESSAGE_ERROR));

      if (GST_MESSAGE_TYPE (msg) != GST_MESSAGE_TAG) /* error or async_done */
        break;

      gst_message_parse_tag (msg, &tags);

      gst_tag_list_foreach (tags, print_one_tag, &md);
      gst_tag_list_unref (tags);

      gst_message_unref (msg);
    };
    gst_element_set_state (pipe, GST_STATE_NULL);
    gst_object_unref (pipe);

    if (GST_MESSAGE_TYPE (msg) == GST_MESSAGE_ERROR) {
        gst_message_unref (msg);
        string msg = "Extracting metadata of file ";
        msg += filename;
        msg += " failed.";
        throw runtime_error(msg);
    }
    title = md.title;
    author = md.author;
    album = md.album;
    gst_message_unref (msg);
    return 0;
}

