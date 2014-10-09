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

#include <mediascanner/MediaFile.hh>
#include <daemon/MetadataExtractor.hh>

#include "test_config.h"

#include<algorithm>
#include<stdexcept>
#include<cstdio>
#include<string>
#include<gst/gst.h>
#include <gtest/gtest.h>

using namespace std;
using namespace mediascanner;

class MetadataExtractorTest : public ::testing::Test {
 protected:
  MetadataExtractorTest() {
  }

  virtual ~MetadataExtractorTest() {
  }

  virtual void SetUp() {
  }

  virtual void TearDown() {
  }
};

TEST_F(MetadataExtractorTest, init) {
    MetadataExtractor extractor;
}

TEST_F(MetadataExtractorTest, detect_audio) {
    MetadataExtractor e;
    string testfile = SOURCE_DIR "/media/testfile.ogg";
    DetectedFile d = e.detect(testfile);
    EXPECT_NE(d.etag, "");
    EXPECT_EQ(d.content_type, "audio/ogg");
    EXPECT_EQ(d.type, AudioMedia);
}

TEST_F(MetadataExtractorTest, detect_video) {
    MetadataExtractor e;
    string testfile = SOURCE_DIR "/media/testvideo_480p.ogv";
    DetectedFile d = e.detect(testfile);
    EXPECT_NE(d.etag, "");
    EXPECT_EQ(d.content_type, "video/ogg");
    EXPECT_EQ(d.type, VideoMedia);
}

TEST_F(MetadataExtractorTest, detect_notmedia) {
    MetadataExtractor e;
    string testfile = SOURCE_DIR "/CMakeLists.txt";
    EXPECT_THROW(e.detect(testfile), runtime_error);
}

TEST_F(MetadataExtractorTest, extract) {
    MetadataExtractor e;
    string testfile = SOURCE_DIR "/media/testfile.ogg";
    MediaFile file = e.extract(e.detect(testfile));

    EXPECT_EQ(file.getType(), AudioMedia);
    EXPECT_EQ(file.getTitle(), "track1");
    EXPECT_EQ(file.getAuthor(), "artist1");
    EXPECT_EQ(file.getAlbum(), "album1");
    EXPECT_EQ(file.getDate(), "2013");
    EXPECT_EQ(file.getTrackNumber(), 1);
    EXPECT_EQ(file.getDuration(), 5);
}

TEST_F(MetadataExtractorTest, extract_video) {
    MetadataExtractor e;

    MediaFile file = e.extract(e.detect(SOURCE_DIR "/media/testvideo_480p.ogv"));
    EXPECT_EQ(file.getType(), VideoMedia);
    EXPECT_EQ(file.getDuration(), 1);
    EXPECT_EQ(file.getWidth(), 854);
    EXPECT_EQ(file.getHeight(), 480);

    file = e.extract(e.detect(SOURCE_DIR "/media/testvideo_720p.ogv"));
    EXPECT_EQ(file.getType(), VideoMedia);
    EXPECT_EQ(file.getDuration(), 1);
    EXPECT_EQ(file.getWidth(), 1280);
    EXPECT_EQ(file.getHeight(), 720);

    file = e.extract(e.detect(SOURCE_DIR "/media/testvideo_1080p.ogv"));
    EXPECT_EQ(file.getType(), VideoMedia);
    EXPECT_EQ(file.getDuration(), 1);
    EXPECT_EQ(file.getWidth(), 1920);
    EXPECT_EQ(file.getHeight(), 1080);
}

TEST_F(MetadataExtractorTest, extract_photo) {
    MetadataExtractor e;

    // An landscape image that should be rotated to portrait
    MediaFile file = e.extract(e.detect(SOURCE_DIR "/media/image1.jpg"));
    EXPECT_EQ(ImageMedia, file.getType());
    EXPECT_EQ(2848, file.getWidth());
    EXPECT_EQ(4272, file.getHeight());
    EXPECT_EQ("2013-01-04T08:25:46", file.getDate());
    EXPECT_DOUBLE_EQ(-28.249409333333336, file.getLatitude());
    EXPECT_DOUBLE_EQ(153.150774, file.getLongitude());

    // A landscape image without rotation.
    file = e.extract(e.detect(SOURCE_DIR "/media/image2.jpg"));
    EXPECT_EQ(ImageMedia, file.getType());
    EXPECT_EQ(4272, file.getWidth());
    EXPECT_EQ(2848, file.getHeight());
    EXPECT_EQ("2013-01-04T09:52:27", file.getDate());
    EXPECT_DOUBLE_EQ(-28.259611, file.getLatitude());
    EXPECT_DOUBLE_EQ(153.1727346, file.getLongitude());
}

// PNG files don't have exif entries, so test we work with those, too.
TEST_F(MetadataExtractorTest, png_file) {
    MetadataExtractor e;
    MediaFile file = e.extract(e.detect(SOURCE_DIR "/media/image3.png"));

    EXPECT_EQ(ImageMedia, file.getType());
    EXPECT_EQ(640, file.getWidth());
    EXPECT_EQ(400, file.getHeight());
    // The time stamp on the test file can be anything. We can't guarantee what it is,
    // so just inspect the format.
    auto timestr = file.getDate();
    EXPECT_EQ(timestr.size(), 19);
    EXPECT_EQ(timestr.find('T'), 10);

    // These can't go inside EXPECT_EQ because it is a macro and mixing templates
    // with macros makes things explode.
    auto dashes = std::count_if(timestr.begin(), timestr.end(), [](char c) { return c == '-';});
    auto colons = std::count_if(timestr.begin(), timestr.end(), [](char c) { return c == ':';});
    EXPECT_EQ(dashes, 2);
    EXPECT_EQ(colons, 2);

    EXPECT_DOUBLE_EQ(0, file.getLatitude());
    EXPECT_DOUBLE_EQ(0, file.getLongitude());

}

int main(int argc, char **argv) {
    gst_init (&argc, &argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
