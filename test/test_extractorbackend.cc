/*
 * Copyright (C) 2014 Canonical, Ltd.
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

#include <mediascanner/MediaFile.hh>
#include <mediascanner/MediaFileBuilder.hh>
#include <extractor/DetectedFile.hh>
#include <extractor/ExtractorBackend.hh>
#include <extractor/GStreamerExtractor.hh>
#include <extractor/TaglibExtractor.hh>

#include "test_config.h"

#include <cstdio>
#include <stdexcept>
#include <string>
#include <gst/gst.h>
#include <gtest/gtest.h>

using namespace std;
using namespace mediascanner;

namespace {

bool supports_decoder(const std::string& format)
{
    typedef std::unique_ptr<GstCaps, decltype(&gst_caps_unref)> CapsPtr;
    static std::vector<CapsPtr> formats;

    if (formats.empty())
    {
        std::unique_ptr<GList, decltype(&gst_plugin_feature_list_free)> decoders(
            gst_element_factory_list_get_elements(GST_ELEMENT_FACTORY_TYPE_DECODER, GST_RANK_NONE),
            gst_plugin_feature_list_free);
        for (const GList* l = decoders.get(); l != nullptr; l = l->next)
        {
            const auto factory = static_cast<GstElementFactory*>(l->data);

            const GList* templates = gst_element_factory_get_static_pad_templates(factory);
            for (const GList* l = templates; l != nullptr; l = l->next)
            {
                const auto t = static_cast<GstStaticPadTemplate*>(l->data);
                if (t->direction != GST_PAD_SINK)
                {
                    continue;
                }
                CapsPtr caps(gst_static_caps_get(&t->static_caps),
                             gst_caps_unref);
                if (gst_caps_is_any(caps.get())) {
                    continue;
                }
                formats.emplace_back(std::move(caps));
            }
        }
    }

    char *end = nullptr;
    GstStructure *structure = gst_structure_from_string(format.c_str(), &end);
    assert(structure != nullptr);
    assert(end == format.c_str() + format.size());
    // GstCaps adopts the GstStructure
    CapsPtr caps(gst_caps_new_full(structure, nullptr), gst_caps_unref);

    for (const auto &other : formats) {
        if (gst_caps_is_always_compatible(caps.get(), other.get())) {
            return true;
        }
    }
    return false;
}

}

class ExtractorBackendTest : public ::testing::Test {
protected:
    ExtractorBackendTest() {
    }

    virtual ~ExtractorBackendTest() {
    }

    virtual void SetUp() override {
    }

    virtual void TearDown() override {
    }
};


TEST_F(ExtractorBackendTest, init) {
    ExtractorBackend extractor;
}

TEST_F(ExtractorBackendTest, extract_vorbis) {
    ExtractorBackend e;
    string testfile = SOURCE_DIR "/media/testfile.ogg";
    DetectedFile df(testfile, "etag", "audio/ogg", 42, AudioMedia);
    MediaFile file = e.extract(df);

    EXPECT_EQ(file.getType(), AudioMedia);
    EXPECT_EQ(file.getTitle(), "track1");
    EXPECT_EQ(file.getAuthor(), "artist1");
    EXPECT_EQ(file.getAlbum(), "album1");
    EXPECT_EQ(file.getDate(), "2013");
    EXPECT_EQ(file.getTrackNumber(), 1);
    EXPECT_EQ(file.getDuration(), 5);
}

TEST_F(ExtractorBackendTest, extract_mp3) {
    ExtractorBackend e;
    string testfile = SOURCE_DIR "/media/testfile.mp3";
    DetectedFile df(testfile, "etag", "audio/mpeg", 42, AudioMedia);
    MediaFile file = e.extract(df);

    EXPECT_EQ(file.getType(), AudioMedia);
    EXPECT_EQ(file.getTitle(), "track1");
    EXPECT_EQ(file.getAuthor(), "artist1");
    EXPECT_EQ(file.getAlbum(), "album1");
    EXPECT_EQ(file.getGenre(), "Hip-Hop");
    EXPECT_EQ(file.getDate(), "2013-06-03");
    EXPECT_EQ(file.getTrackNumber(), 1);
    EXPECT_EQ(file.getDuration(), 1);
}

TEST_F(ExtractorBackendTest, extract_m4a) {
    ExtractorBackend e;
    string testfile = SOURCE_DIR "/media/testfile.m4a";
    DetectedFile df(testfile, "etag", "audio/mpeg4", 42, AudioMedia);
    MediaFile file = e.extract(df);

    EXPECT_EQ(file.getType(), AudioMedia);
    EXPECT_EQ(file.getTitle(), "Title");
    EXPECT_EQ(file.getAuthor(), "Artist");
    EXPECT_EQ(file.getAlbum(), "Album");
    EXPECT_EQ(file.getGenre(), "Rock");
    EXPECT_EQ(file.getDate(), "2015-10-07");
    EXPECT_EQ(file.getTrackNumber(), 4);
    EXPECT_EQ(file.getDuration(), 1);
}

TEST_F(ExtractorBackendTest, extract_video) {
    ExtractorBackend e;

    MediaFile file = e.extract(DetectedFile(
        SOURCE_DIR "/media/testvideo_480p.ogv", "etag", "video/ogg", 42, VideoMedia));
    EXPECT_EQ(file.getType(), VideoMedia);
    EXPECT_EQ(file.getDuration(), 1);
    EXPECT_EQ(file.getWidth(), 854);
    EXPECT_EQ(file.getHeight(), 480);

    file = e.extract(DetectedFile(
        SOURCE_DIR "/media/testvideo_720p.ogv", "etag", "video/ogg", 42, VideoMedia));
    EXPECT_EQ(file.getType(), VideoMedia);
    EXPECT_EQ(file.getDuration(), 1);
    EXPECT_EQ(file.getWidth(), 1280);
    EXPECT_EQ(file.getHeight(), 720);

    file = e.extract(DetectedFile(
        SOURCE_DIR "/media/testvideo_1080p.ogv", "etag", "video/ogg", 42, VideoMedia));
    EXPECT_EQ(file.getType(), VideoMedia);
    EXPECT_EQ(file.getDuration(), 1);
    EXPECT_EQ(file.getWidth(), 1920);
    EXPECT_EQ(file.getHeight(), 1080);
}

TEST_F(ExtractorBackendTest, extract_photo) {
    ExtractorBackend e;

    // An landscape image that should be rotated to portrait
    MediaFile file = e.extract(DetectedFile(
        SOURCE_DIR "/media/image1.jpg", "etag", "image/jpeg", 42, ImageMedia));
    EXPECT_EQ(ImageMedia, file.getType());
    EXPECT_EQ(2848, file.getWidth());
    EXPECT_EQ(4272, file.getHeight());
    EXPECT_EQ("2013-01-04T08:25:46", file.getDate());
    EXPECT_DOUBLE_EQ(-28.249409333333336, file.getLatitude());
    EXPECT_DOUBLE_EQ(153.150774, file.getLongitude());

    // A landscape image without rotation.
    file = e.extract(DetectedFile(
        SOURCE_DIR "/media/image2.jpg", "etag", "image/jpeg", 42, ImageMedia));
    EXPECT_EQ(ImageMedia, file.getType());
    EXPECT_EQ(4272, file.getWidth());
    EXPECT_EQ(2848, file.getHeight());
    EXPECT_EQ("2013-01-04T09:52:27", file.getDate());
    EXPECT_DOUBLE_EQ(-28.259611, file.getLatitude());
    EXPECT_DOUBLE_EQ(153.1727346, file.getLongitude());
}

TEST_F(ExtractorBackendTest, extract_photo_date_original) {
    ExtractorBackend e;

    MediaFile file = e.extract(DetectedFile(
        SOURCE_DIR "/media/krillin.jpg", "etag", "image/jpeg", 42, ImageMedia));
    EXPECT_EQ(ImageMedia, file.getType());
    EXPECT_EQ("2016-01-22T18:28:42", file.getDate());
}

void compare_taglib_gst(const DetectedFile d) {
    GStreamerExtractor gst(5);
    MediaFileBuilder builder_gst(d.filename);
    gst.extract(d, builder_gst);

    TaglibExtractor taglib;
    MediaFileBuilder builder_taglib(d.filename);
    ASSERT_TRUE(taglib.extract(d, builder_taglib));

    MediaFile media_gst(builder_gst);
    MediaFile media_taglib(builder_taglib);
    EXPECT_EQ(media_gst, media_taglib);

    // And check individual keys to improve error handling:
    EXPECT_EQ(media_gst.getTitle(), media_taglib.getTitle());
    EXPECT_EQ(media_gst.getAuthor(), media_taglib.getAuthor());
    EXPECT_EQ(media_gst.getAlbum(), media_taglib.getAlbum());
    EXPECT_EQ(media_gst.getAlbumArtist(), media_taglib.getAlbumArtist());
    EXPECT_EQ(media_gst.getDate(), media_taglib.getDate());
    EXPECT_EQ(media_gst.getGenre(), media_taglib.getGenre());
    EXPECT_EQ(media_gst.getDiscNumber(), media_taglib.getDiscNumber());
    EXPECT_EQ(media_gst.getTrackNumber(), media_taglib.getTrackNumber());
    EXPECT_EQ(media_gst.getDuration(), media_taglib.getDuration());
    EXPECT_EQ(media_gst.getHasThumbnail(), media_taglib.getHasThumbnail());
}

TEST_F(ExtractorBackendTest, check_taglib_gst_vorbis) {
    DetectedFile d(SOURCE_DIR "/media/testfile.ogg", "etag", "audio/ogg", 42, AudioMedia);
    compare_taglib_gst(d);
}

TEST_F(ExtractorBackendTest, check_taglib_gst_mp3) {
    if (!supports_decoder("audio/mpeg, mpegversion=(int)1, layer=(int)3")) {
        printf("MP3 codec not supported\n");
        return;
    }
    DetectedFile d(SOURCE_DIR "/media/testfile.mp3", "etag", "audio/mpeg", 42, AudioMedia);
    compare_taglib_gst(d);
}

TEST_F(ExtractorBackendTest, check_taglib_gst_m4a) {
    if (!supports_decoder("audio/mpeg, mpegversion=(int)4, stream-format=(string)raw")) {
        printf("M4A codec not supported\n");
        return;
    }
    DetectedFile d(SOURCE_DIR "/media/testfile.m4a", "etag", "audio/mp4", 42, AudioMedia);
    compare_taglib_gst(d);
}


int main(int argc, char **argv) {
    gst_init(&argc, &argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
