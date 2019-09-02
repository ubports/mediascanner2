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

#include "test_config.h"
#include "mediascanner/Album.hh"
#include "mediascanner/MediaFile.hh"
#include "mediascanner/MediaFileBuilder.hh"

#include <gtest/gtest.h>

#include <chrono>
#include <cstdlib>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdexcept>
#include <thread>
#include <vector>

using namespace mediascanner;

class MFBTest : public ::testing::Test {
protected:
    MFBTest() = default;
    virtual ~MFBTest() = default;

    virtual void SetUp() override {
        tmpdir = TEST_DIR "/mfbuilder-test.XXXXXX";
        ASSERT_NE(nullptr, mkdtemp(&tmpdir[0]));
    }

    virtual void TearDown() override {
        if (!tmpdir.empty()) {
            std::string cmd = "rm -rf " + tmpdir;
            ASSERT_EQ(0, system(cmd.c_str()));
        }
    }

    void touch(const std::string &fname, bool sleep=false) {
        if (sleep) {
            // Ensure time stamps change
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        int fd = open(fname.c_str(), O_CREAT, 0600);
        ASSERT_GT(fd, 0);
        ASSERT_EQ(0, close(fd));
    }

    void remove(const std::string &fname, bool sleep=false) {
        if (sleep) {
            // Ensure time stamps change
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        ASSERT_EQ(0, unlink(fname.c_str()));
    }

    std::string tmpdir;
};

TEST_F(MFBTest, basic) {
    MediaType type(AudioMedia);
    std::string fname("abc");
    std::string title("def");
    std::string date("ghi");
    std::string author("jkl");
    std::string album("mno");
    std::string album_artist("pqr");
    std::string etag("stu");
    std::string content_type("vwx");
    std::string genre("yz");
    int disc_number = 2;
    int track_number = 13;
    int duration = 99;
    int width = 640;
    int height = 480;
    double latitude = 67.2;
    double longitude = -7.5;
    uint64_t mtime = 42;

    MediaFileBuilder b(fname);

    b.setType(type);
    b.setTitle(title);
    b.setDate(date);
    b.setAuthor(author);
    b.setAlbum(album);
    b.setAlbumArtist(album_artist);
    b.setGenre(genre);
    b.setDiscNumber(disc_number);
    b.setTrackNumber(track_number);
    b.setDuration(duration);
    b.setETag(etag);
    b.setContentType(content_type);
    b.setWidth(width);
    b.setHeight(height);
    b.setLatitude(latitude);
    b.setLongitude(longitude);
    b.setModificationTime(mtime);

    // Now see if data survives a round trip.
    MediaFile mf = b.build();
    EXPECT_EQ(mf.getType(), type);
    EXPECT_EQ(mf.getFileName(), fname);
    EXPECT_EQ(mf.getTitle(), title);
    EXPECT_EQ(mf.getDate(), date);
    EXPECT_EQ(mf.getAuthor(), author);
    EXPECT_EQ(mf.getAlbum(), album);
    EXPECT_EQ(mf.getAlbumArtist(), album_artist);
    EXPECT_EQ(mf.getGenre(), genre);
    EXPECT_EQ(mf.getDiscNumber(), disc_number);
    EXPECT_EQ(mf.getTrackNumber(), track_number);
    EXPECT_EQ(mf.getDuration(), duration);
    EXPECT_EQ(mf.getETag(), etag);
    EXPECT_EQ(mf.getContentType(), content_type);
    EXPECT_EQ(mf.getWidth(), width);
    EXPECT_EQ(mf.getHeight(), height);
    EXPECT_DOUBLE_EQ(mf.getLatitude(), latitude);
    EXPECT_DOUBLE_EQ(mf.getLongitude(), longitude);
    EXPECT_EQ(mf.getModificationTime(), mtime);

    MediaFileBuilder mfb2(mf);
    MediaFile mf2 = mfb2.build();
    EXPECT_EQ(mf, mf2);
}

TEST_F(MFBTest, chaining) {
    MediaType type(AudioMedia);
    std::string fname("abc");
    std::string title("def");
    std::string date("ghi");
    std::string author("jkl");
    std::string album("mno");
    std::string album_artist("pqr");
    std::string etag("stu");
    std::string content_type("vwx");
    std::string genre("yz");
    int disc_number = 2;
    int track_number = 13;
    int duration = 99;
    int width = 640;
    int height = 480;
    double latitude = 67.2;
    double longitude = -7.5;
    uint64_t mtime = 42;

    MediaFile mf = MediaFileBuilder(fname)
        .setType(type)
        .setTitle(title)
        .setDate(date)
        .setAuthor(author)
        .setAlbum(album)
        .setAlbumArtist(album_artist)
        .setGenre(genre)
        .setDiscNumber(disc_number)
        .setTrackNumber(track_number)
        .setDuration(duration)
        .setWidth(width)
        .setHeight(height)
        .setLatitude(latitude)
        .setLongitude(longitude)
        .setETag(etag)
        .setContentType(content_type)
        .setModificationTime(42);

    // Now see if data survives a round trip.
    EXPECT_EQ(mf.getType(), type);
    EXPECT_EQ(mf.getFileName(), fname);
    EXPECT_EQ(mf.getTitle(), title);
    EXPECT_EQ(mf.getDate(), date);
    EXPECT_EQ(mf.getAuthor(), author);
    EXPECT_EQ(mf.getAlbum(), album);
    EXPECT_EQ(mf.getAlbumArtist(), album_artist);
    EXPECT_EQ(mf.getGenre(), genre);
    EXPECT_EQ(mf.getDiscNumber(), disc_number);
    EXPECT_EQ(mf.getTrackNumber(), track_number);
    EXPECT_EQ(mf.getDuration(), duration);
    EXPECT_EQ(mf.getETag(), etag);
    EXPECT_EQ(mf.getContentType(), content_type);
    EXPECT_EQ(mf.getWidth(), width);
    EXPECT_EQ(mf.getHeight(), height);
    EXPECT_DOUBLE_EQ(mf.getLatitude(), latitude);
    EXPECT_DOUBLE_EQ(mf.getLongitude(), longitude);
    EXPECT_EQ(mf.getModificationTime(), mtime);
}

TEST_F(MFBTest, fallback_title) {
    // Fallback title is derived from file name.
    MediaFile mf = MediaFileBuilder("/path/to/abc.ogg");
    EXPECT_EQ(mf.getTitle(), "abc");
}

TEST_F(MFBTest, fallback_album_artist) {
    // Fallback album_artist is the author.
    MediaFile mf = MediaFileBuilder("abc")
        .setAuthor("author");
    EXPECT_EQ(mf.getAlbumArtist(), "author");
}

TEST_F(MFBTest, faulty_usage) {
    MediaFileBuilder mfb("/foo/bar/baz.mp3");
    MediaFile m1(std::move(mfb));
    ASSERT_THROW(MediaFile m2(std::move(mfb)), std::logic_error);
    ASSERT_THROW(MediaFile m3(mfb), std::logic_error);
}

TEST_F(MFBTest, album_art_uri) {
    // No embedded art: use external art fetcher
    MediaFile mf = MediaFileBuilder("/foo/bar/baz.mp3")
        .setType(AudioMedia)
        .setAuthor("The Artist")
        .setAlbum("The Album");
    EXPECT_EQ("image://albumart/artist=The%20Artist&album=The%20Album", mf.getArtUri());

    // Embedded art: use thumbnailer
    mf = MediaFileBuilder("/foo/bar/baz.mp3")
        .setType(AudioMedia)
        .setAuthor("The Artist")
        .setAlbum("The Album")
        .setHasThumbnail(true);
    EXPECT_EQ("image://thumbnailer/file:///foo/bar/baz.mp3", mf.getArtUri());

    // Videos use thumbnailer
    mf = MediaFileBuilder("/foo/bar/baz.mp4")
        .setType(VideoMedia)
        .setAuthor("The Artist")
        .setAlbum("The Album");
    EXPECT_EQ("image://thumbnailer/file:///foo/bar/baz.mp4", mf.getArtUri());
}

TEST_F(MFBTest, folder_art) {
    std::string fname = tmpdir + "/dummy.mp3";
    MediaFile media = MediaFileBuilder(fname)
        .setType(AudioMedia)
        .setTitle("Title")
        .setAuthor("Artist")
        .setAlbum("Album");

    EXPECT_EQ("image://albumart/artist=Artist&album=Album", media.getArtUri());
    touch(tmpdir + "/folder.jpg", true);
    EXPECT_NE(std::string::npos, media.getArtUri().find("/folder.jpg")) << media.getArtUri();
    remove(tmpdir + "/folder.jpg", true);
    EXPECT_EQ("image://albumart/artist=Artist&album=Album", media.getArtUri());
}

TEST_F(MFBTest, folder_art_case_insensitive) {
   std::string fname = tmpdir + "/dummy.mp3";
    MediaFile media = MediaFileBuilder(fname)
        .setType(AudioMedia)
        .setTitle("Title")
        .setAuthor("Artist")
        .setAlbum("Album");

    touch(tmpdir + "/FOLDER.JPG");
    EXPECT_NE(std::string::npos, media.getArtUri().find("/FOLDER.JPG")) << media.getArtUri();
}

TEST_F(MFBTest, folder_art_precedence) {
    std::string fname = tmpdir + "/dummy.mp3";
    MediaFile media = MediaFileBuilder(fname)
        .setType(AudioMedia)
        .setTitle("Title")
        .setAuthor("Artist")
        .setAlbum("Album");

    touch(tmpdir + "/cover.jpg");
    touch(tmpdir + "/album.jpg");
    touch(tmpdir + "/albumart.jpg");
    touch(tmpdir + "/.folder.jpg");
    touch(tmpdir + "/folder.jpeg");
    touch(tmpdir + "/folder.jpg");
    touch(tmpdir + "/folder.png");

    EXPECT_NE(std::string::npos, media.getArtUri().find("/cover.jpg")) << media.getArtUri();
    remove(tmpdir + "/cover.jpg", true);

    EXPECT_NE(std::string::npos, media.getArtUri().find("/album.jpg")) << media.getArtUri();
    remove(tmpdir + "/album.jpg", true);

    EXPECT_NE(std::string::npos, media.getArtUri().find("/albumart.jpg")) << media.getArtUri();
    remove(tmpdir + "/albumart.jpg", true);

    EXPECT_NE(std::string::npos, media.getArtUri().find("/.folder.jpg")) << media.getArtUri();
    remove(tmpdir + "/.folder.jpg", true);

    EXPECT_NE(std::string::npos, media.getArtUri().find("/folder.jpeg")) << media.getArtUri();
    remove(tmpdir + "/folder.jpeg", true);

    EXPECT_NE(std::string::npos, media.getArtUri().find("/folder.jpg")) << media.getArtUri();
    remove(tmpdir + "/folder.jpg", true);

    EXPECT_NE(std::string::npos, media.getArtUri().find("/folder.png")) << media.getArtUri();
}

TEST_F(MFBTest, folder_art_cache_coverage) {
    std::vector<MediaFile> files;
    for (int i = 0; i < 100; i++) {
        std::string directory = tmpdir + "/" + std::to_string(i);
        ASSERT_EQ(0, mkdir(directory.c_str(), 0700));
        touch(directory + "/folder.jpg");

        std::string fname = directory + "/dummy.mp3";
        files.emplace_back(MediaFileBuilder(fname)
                           .setType(AudioMedia)
                           .setTitle("Title")
                           .setAuthor("Artist")
                           .setAlbum("Album"));
    }

    // Check art for a number of files smaller than the cache size twice
    for (int i = 0; i < 10; i++) {
        const auto &media = files[i];
        EXPECT_NE(std::string::npos, media.getArtUri().find("/folder.jpg")) << media.getArtUri();
    }
    for (int i = 0; i < 10; i++) {
        const auto &media = files[i];
        EXPECT_NE(std::string::npos, media.getArtUri().find("/folder.jpg")) << media.getArtUri();
    }

    // Now check a larger number of files twice
    for (const auto &media : files) {
        EXPECT_NE(std::string::npos, media.getArtUri().find("/folder.jpg")) << media.getArtUri();
    }
    for (const auto &media : files) {
        EXPECT_NE(std::string::npos, media.getArtUri().find("/folder.jpg")) << media.getArtUri();
    }
}

TEST_F(MFBTest, album_art) {
    std::string fname = tmpdir + "/dummy.mp3";

    // File with embedded art
    Album album("Album", "Artist", "2015-11-23", "Rock", fname, true, 1);
    EXPECT_NE(std::string::npos, album.getArtUri().find("/dummy.mp3")) << album.getArtUri();

    // No embedded art
    album = Album("Album", "Artist", "2015-11-23", "Rock", fname, false, 1);
    EXPECT_EQ("image://albumart/artist=Artist&album=Album", album.getArtUri());

    // No embedded art, but folder art available
    touch(tmpdir + "/folder.jpg", true);
    EXPECT_NE(std::string::npos, album.getArtUri().find("/folder.jpg")) << album.getArtUri();
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
