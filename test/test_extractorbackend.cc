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
#include <extractor/DetectedFile.hh>
#include <extractor/ExtractorBackend.hh>

#include "test_config.h"

#include <cstdio>
#include <stdexcept>
#include <string>
#include <gst/gst.h>
#include <gtest/gtest.h>

using namespace std;
using namespace mediascanner;

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

TEST_F(ExtractorBackendTest, extract_audio) {
    ExtractorBackend e;
    string testfile = SOURCE_DIR "/media/testfile.ogg";
    DetectedFile df(testfile, "etag", "audio/ogg", AudioMedia);
    MediaFile file = e.extract(df);

    EXPECT_EQ(file.getType(), AudioMedia);
    EXPECT_EQ(file.getTitle(), "track1");
    EXPECT_EQ(file.getAuthor(), "artist1");
    EXPECT_EQ(file.getAlbum(), "album1");
    EXPECT_EQ(file.getDate(), "2013");
    EXPECT_EQ(file.getTrackNumber(), 1);
    EXPECT_EQ(file.getDuration(), 5);
}

TEST_F(ExtractorBackendTest, extract_video) {
    ExtractorBackend e;

    MediaFile file = e.extract(DetectedFile(
        SOURCE_DIR "/media/testvideo_480p.ogv", "etag", "video/ogg", VideoMedia));
    EXPECT_EQ(file.getType(), VideoMedia);
    EXPECT_EQ(file.getDuration(), 1);
    EXPECT_EQ(file.getWidth(), 854);
    EXPECT_EQ(file.getHeight(), 480);

    file = e.extract(DetectedFile(
        SOURCE_DIR "/media/testvideo_720p.ogv", "etag", "video/ogg", VideoMedia));
    EXPECT_EQ(file.getType(), VideoMedia);
    EXPECT_EQ(file.getDuration(), 1);
    EXPECT_EQ(file.getWidth(), 1280);
    EXPECT_EQ(file.getHeight(), 720);

    file = e.extract(DetectedFile(
        SOURCE_DIR "/media/testvideo_1080p.ogv", "etag", "video/ogg", VideoMedia));
    EXPECT_EQ(file.getType(), VideoMedia);
    EXPECT_EQ(file.getDuration(), 1);
    EXPECT_EQ(file.getWidth(), 1920);
    EXPECT_EQ(file.getHeight(), 1080);
}

TEST_F(ExtractorBackendTest, extract_photo) {
    ExtractorBackend e;

    // An landscape image that should be rotated to portrait
    MediaFile file = e.extract(DetectedFile(
        SOURCE_DIR "/media/image1.jpg", "etag", "image/jpeg", ImageMedia));
    EXPECT_EQ(ImageMedia, file.getType());
    EXPECT_EQ(2848, file.getWidth());
    EXPECT_EQ(4272, file.getHeight());
    EXPECT_EQ("2013-01-04T08:25:46", file.getDate());
    EXPECT_DOUBLE_EQ(-28.249409333333336, file.getLatitude());
    EXPECT_DOUBLE_EQ(153.150774, file.getLongitude());

    // A landscape image without rotation.
    file = e.extract(DetectedFile(
        SOURCE_DIR "/media/image2.jpg", "etag", "image/jpeg", ImageMedia));
    EXPECT_EQ(ImageMedia, file.getType());
    EXPECT_EQ(4272, file.getWidth());
    EXPECT_EQ(2848, file.getHeight());
    EXPECT_EQ("2013-01-04T09:52:27", file.getDate());
    EXPECT_DOUBLE_EQ(-28.259611, file.getLatitude());
    EXPECT_DOUBLE_EQ(153.1727346, file.getLongitude());
}

int main(int argc, char **argv) {
    gst_init(&argc, &argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
