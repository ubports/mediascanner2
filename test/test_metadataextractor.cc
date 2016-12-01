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

#include <mediascanner/MediaFile.hh>
#include <extractor/DetectedFile.hh>
#include <extractor/MetadataExtractor.hh>

#include "test_config.h"

#include <algorithm>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>

#include <gio/gio.h>
#include <gtest/gtest.h>

using namespace std;
using namespace mediascanner;

class MetadataExtractorTest : public ::testing::Test {
protected:
    virtual void SetUp() override {
        test_dbus_.reset(g_test_dbus_new(G_TEST_DBUS_NONE));
        g_test_dbus_add_service_dir(test_dbus_.get(), TEST_DIR "/services");
    }

    virtual void TearDown() override {
        session_bus_.reset();
        test_dbus_.reset();
        unsetenv("MEDIASCANNER_EXTRACTOR_CRASH_AFTER");
    }

    GDBusConnection *session_bus() {
        if (!bus_started_) {
            g_test_dbus_up(test_dbus_.get());

            GError *error = nullptr;
            char *address = g_dbus_address_get_for_bus_sync(G_BUS_TYPE_SESSION, nullptr, &error);
            if (!address) {
                std::string errortxt(error->message);
                g_error_free(error);
                throw std::runtime_error(
                    std::string("Failed to determine session bus address: ") + errortxt);
            }
            session_bus_.reset(g_dbus_connection_new_for_address_sync(
                address, static_cast<GDBusConnectionFlags>(G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT | G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION), nullptr, nullptr, &error));
            g_free(address);
            if (!session_bus_) {
                std::string errortxt(error->message);
                g_error_free(error);
                throw std::runtime_error(
                    std::string("Failed to connect to session bus: ") + errortxt);
            }
            bus_started_ = true;
        }
        return session_bus_.get();
    }

private:
    unique_ptr<GTestDBus,decltype(&g_object_unref)> test_dbus_ {nullptr, g_object_unref};
    unique_ptr<GDBusConnection,decltype(&g_object_unref)> session_bus_ {nullptr, g_object_unref};
    bool bus_started_ = false;
};


TEST_F(MetadataExtractorTest, init) {
    MetadataExtractor extractor(session_bus());
}

TEST_F(MetadataExtractorTest, detect_audio) {
    MetadataExtractor e(session_bus());
    string testfile = SOURCE_DIR "/media/testfile.ogg";
    DetectedFile d = e.detect(testfile);
    EXPECT_NE(d.etag, "");
    EXPECT_EQ(d.content_type, "audio/ogg");
    EXPECT_EQ(d.type, AudioMedia);

    struct stat st;
    ASSERT_EQ(0, stat(testfile.c_str(), &st));
    EXPECT_EQ(st.st_mtime, d.mtime);
}

TEST_F(MetadataExtractorTest, detect_video) {
    MetadataExtractor e(session_bus());
    string testfile = SOURCE_DIR "/media/testvideo_480p.ogv";
    DetectedFile d = e.detect(testfile);
    EXPECT_NE(d.etag, "");
    EXPECT_EQ(d.content_type, "video/ogg");
    EXPECT_EQ(d.type, VideoMedia);

    struct stat st;
    ASSERT_EQ(0, stat(testfile.c_str(), &st));
    EXPECT_EQ(st.st_mtime, d.mtime);
}

TEST_F(MetadataExtractorTest, detect_notmedia) {
    MetadataExtractor e(session_bus());
    string testfile = SOURCE_DIR "/CMakeLists.txt";
    EXPECT_THROW(e.detect(testfile), runtime_error);
}

TEST_F(MetadataExtractorTest, extract) {
    MetadataExtractor e(session_bus());
    string testfile = SOURCE_DIR "/media/testfile.ogg";
    MediaFile file = e.extract(e.detect(testfile));

    EXPECT_EQ(file.getType(), AudioMedia);
    EXPECT_EQ(file.getTitle(), "track1");
    EXPECT_EQ(file.getAuthor(), "artist1");
    EXPECT_EQ(file.getAlbum(), "album1");
    EXPECT_EQ(file.getDate(), "2013");
    EXPECT_EQ(file.getTrackNumber(), 1);
    EXPECT_EQ(file.getDuration(), 5);
    EXPECT_EQ(file.getHasThumbnail(), false);
}

TEST_F(MetadataExtractorTest, extract_vorbis_art) {
    MetadataExtractor e(session_bus());
    string testfile = SOURCE_DIR "/media/embedded-art.ogg";
    MediaFile file = e.extract(e.detect(testfile));

    EXPECT_EQ(file.getHasThumbnail(), true);
}

TEST_F(MetadataExtractorTest, extract_mp3) {
    MetadataExtractor e(session_bus());
    string testfile = SOURCE_DIR "/media/testfile.mp3";
    MediaFile file = e.extract(e.detect(testfile));

    EXPECT_EQ(file.getType(), AudioMedia);
    EXPECT_EQ(file.getTitle(), "track1");
    EXPECT_EQ(file.getAuthor(), "artist1");
    EXPECT_EQ(file.getAlbum(), "album1");
    EXPECT_EQ(file.getDate(), "2013-06-03");
    EXPECT_EQ(file.getTrackNumber(), 1);
    EXPECT_EQ(file.getDuration(), 1);
    EXPECT_EQ(file.getGenre(), "Hip-Hop");
    struct stat st;
    ASSERT_EQ(0, stat(testfile.c_str(), &st));
    EXPECT_EQ(st.st_mtime, file.getModificationTime());
}

TEST_F(MetadataExtractorTest, extract_m4a) {
    MetadataExtractor e(session_bus());
    string testfile = SOURCE_DIR "/media/testfile.m4a";
    MediaFile file = e.extract(e.detect(testfile));

    EXPECT_EQ(AudioMedia, file.getType());
    EXPECT_EQ("Title", file.getTitle());
    EXPECT_EQ("Artist", file.getAuthor());
    EXPECT_EQ("Album", file.getAlbum());
    EXPECT_EQ("Album Artist", file.getAlbumArtist());
    EXPECT_EQ("2015-10-07", file.getDate());
    EXPECT_EQ(4, file.getTrackNumber());
    EXPECT_EQ(1, file.getDiscNumber());
    EXPECT_EQ(1, file.getDuration());
    EXPECT_EQ("Rock", file.getGenre());
    EXPECT_EQ(true, file.getHasThumbnail());
    struct stat st;
    ASSERT_EQ(0, stat(testfile.c_str(), &st));
    EXPECT_EQ(st.st_mtime, file.getModificationTime());
}

TEST_F(MetadataExtractorTest, extract_video) {
    MetadataExtractor e(session_bus());

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
    MetadataExtractor e(session_bus());

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

TEST_F(MetadataExtractorTest, extract_bad_date) {
    MetadataExtractor e(session_bus());
    string testfile = SOURCE_DIR "/media/baddate.ogg";
    MediaFile file = e.extract(e.detect(testfile));

    EXPECT_EQ(file.getType(), AudioMedia);
    EXPECT_EQ(file.getTitle(), "Track");
    EXPECT_EQ(file.getAuthor(), "Artist");
    EXPECT_EQ(file.getAlbum(), "Album");
    EXPECT_EQ(file.getDate(), "");
}

TEST_F(MetadataExtractorTest, extract_mp3_bad_date) {
    MetadataExtractor e(session_bus());
    string testfile = SOURCE_DIR "/media/baddate.mp3";
    MediaFile file = e.extract(e.detect(testfile));

    EXPECT_EQ(file.getType(), AudioMedia);
    EXPECT_EQ(file.getTitle(), "Track");
    EXPECT_EQ(file.getAuthor(), "Artist");
    EXPECT_EQ(file.getAlbum(), "Album");
    EXPECT_EQ(file.getDate(), "");
}

TEST_F(MetadataExtractorTest, blacklist) {
    MetadataExtractor e(session_bus());
    string testfile = SOURCE_DIR "/media/playlist.m3u";
    try {
        e.detect(testfile);
        FAIL();
    } catch(const std::runtime_error &e) {
        std::string error_message(e.what());
        ASSERT_NE(error_message.find("blacklist"), std::string::npos);
    }
}

TEST_F(MetadataExtractorTest, png_file) {
    // PNG files don't have exif entries, so test we work with those, too.
    MetadataExtractor e(session_bus());
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

TEST_F(MetadataExtractorTest, extractor_crash) {
    setenv("MEDIASCANNER_EXTRACTOR_CRASH_AFTER", "0", true);

    MetadataExtractor e(session_bus());
    string testfile = SOURCE_DIR "/media/testfile.ogg";
    try {
        MediaFile file = e.extract(e.detect(testfile));
        FAIL();
    } catch (const std::runtime_error &e) {
        EXPECT_NE(std::string::npos, std::string(e.what()).find("ExtractMetadata D-Bus call failed")) << e.what();
    }
}

TEST_F(MetadataExtractorTest, crash_recovery) {
    setenv("MEDIASCANNER_EXTRACTOR_CRASH_AFTER", "1", true);

    MetadataExtractor e(session_bus());
    string testfile = SOURCE_DIR "/media/testfile.ogg";
    // First extraction succeeds
    MediaFile file = e.extract(e.detect(testfile));

    // Second try succeeds, with the extraction daemon being
    // restarted.
    file = e.extract(e.detect(testfile));
    EXPECT_EQ("track1", file.getTitle());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
