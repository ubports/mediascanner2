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
#include <mediascanner/Album.hh>
#include <mediascanner/MediaStore.hh>
#include <mediascanner/utils.hh>

#include<stdexcept>
#include<cstdio>
#include<string>
#include <gtest/gtest.h>

using namespace std;

class MediaStoreTest : public ::testing::Test {
 protected:
  MediaStoreTest() {
  }

  virtual ~MediaStoreTest() {
  }

  virtual void SetUp() {
  }

  virtual void TearDown() {
  }
};

TEST_F(MediaStoreTest, init) {
    MediaStore store(":memory:", MS_READ_WRITE);
}

TEST_F(MediaStoreTest, equality) {
    MediaFile audio1("a", "1900", "b", "c", "d", "e", 1, 5, AudioMedia);
    MediaFile audio2("aa", "1900", "b", "c", "d", "e", 1, 5, AudioMedia);

    MediaFile video1("a", "b", "1900", "c", "d", "e", 0, 5, VideoMedia);
    MediaFile video2("aa", "b", "1900", "c", "d", "e", 0, 5, VideoMedia);

    EXPECT_EQ(audio1, audio1);
    EXPECT_EQ(video1, video1);

    EXPECT_NE(audio1, audio2);
    EXPECT_NE(audio1, video1);
    EXPECT_NE(audio2, video1);
    EXPECT_NE(audio2, video2);
}

TEST_F(MediaStoreTest, roundtrip) {
    MediaFile audio("aaa", "bbb bbb", "1900-01-01", "ccc", "ddd", "eee", 3, 5, AudioMedia);
    MediaFile video("aaa2", "bbb bbb", "2012-01-01", "ccc", "ddd", "eee", 0, 5, VideoMedia);
    MediaStore store(":memory:", MS_READ_WRITE);
    store.insert(audio);
    store.insert(video);
    vector<MediaFile> result = store.query("bbb", AudioMedia);
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], audio);
    result = store.query("bbb", VideoMedia);
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], video);
}

TEST_F(MediaStoreTest, query_by_album) {
   MediaFile audio("/path/foo.ogg", "title", "1900-01-01", "artist", "album", "albumartist", 3, 5, AudioMedia);
    MediaStore store(":memory:", MS_READ_WRITE);
    store.insert(audio);

    vector<MediaFile> result = store.query("album", AudioMedia);
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], audio);
 }

TEST_F(MediaStoreTest, query_by_artist) {
   MediaFile audio("/path/foo.ogg", "title", "1900-01-01", "artist", "album", "albumartist", 3, 5, AudioMedia);
    MediaStore store(":memory:", MS_READ_WRITE);
    store.insert(audio);

    vector<MediaFile> result = store.query("artist", AudioMedia);
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], audio);
 }

TEST_F(MediaStoreTest, query_ranking) {
   MediaFile audio1("/path/foo1.ogg", "title", "1900-01-01", "artist", "album", "albumartist", 3, 5, AudioMedia);
   MediaFile audio2("/path/foo2.ogg", "title aaa", "1900-01-01", "artist", "album", "albumartist", 3, 5, AudioMedia);
   MediaFile audio3("/path/foo3.ogg", "title", "1900-01-01", "artist aaa", "album", "albumartist", 3, 5, AudioMedia);
   MediaFile audio4("/path/foo4.ogg", "title", "1900-01-01", "artist", "album aaa", "albumartist", 3, 5, AudioMedia);
   MediaFile audio5("/path/foo5.ogg", "title aaa", "1900-01-01", "artist aaa", "album aaa", "albumartist", 3, 5, AudioMedia);

   MediaStore store(":memory:", MS_READ_WRITE);
   store.insert(audio1);
   store.insert(audio2);
   store.insert(audio3);
   store.insert(audio4);
   store.insert(audio5);

    vector<MediaFile> result = store.query("aaa", AudioMedia);
    ASSERT_EQ(result.size(), 4);
    EXPECT_EQ(result[0], audio5); // Term appears in title, artist and album
    EXPECT_EQ(result[1], audio2); // title has highest weighting
    EXPECT_EQ(result[2], audio4); // then album
    EXPECT_EQ(result[3], audio3); // then artist
}

TEST_F(MediaStoreTest, unmount) {
    MediaFile audio1("/media/username/dir/fname.ogg", "bbb bbb", "2000-01-01", "ccc", "ddd", "eee", 1, 5, AudioMedia);
    MediaFile audio2("/home/username/Music/fname.ogg", "bbb bbb", "1900-01-01", "ccc", "ddd", "eee", 42, 5, AudioMedia);
    MediaStore store(":memory:", MS_READ_WRITE);
    store.insert(audio1);
    store.insert(audio2);
    vector<MediaFile> result = store.query("bbb", AudioMedia);
    ASSERT_EQ(result.size(), 2);

    store.archiveItems("/media/username");
    result = store.query("bbb", AudioMedia);
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], audio2);

    store.restoreItems("/media/username");
    result = store.query("bbb", AudioMedia);
    ASSERT_EQ(result.size(), 2);
}

TEST_F(MediaStoreTest, utils) {
    string source("_a.b(c)[d]{e}f.mp3");
    string correct = {" a b c  d  e f"};
    string result = filenameToTitle(source);
    EXPECT_EQ(correct, result);

    string unquoted(R"(It's a living.)");
    string quoted(R"('It''s a living.')");
    EXPECT_EQ(sqlQuote(unquoted), quoted);
}

TEST_F(MediaStoreTest, queryAlbums) {
    MediaFile audio1("/home/username/Music/track1.ogg", "TitleOne", "1900-01-01", "ArtistOne", "AlbumOne", "Various Artists", 1, 5, AudioMedia);
    MediaFile audio2("/home/username/Music/track2.ogg", "TitleTwo", "1900-01-01", "ArtistTwo", "AlbumOne", "Various Artists", 2, 5, AudioMedia);
    MediaFile audio3("/home/username/Music/track3.ogg", "TitleThree", "1900-01-01", "ArtistThree", "AlbumOne", "Various Artists", 3, 5, AudioMedia);
    MediaFile audio4("/home/username/Music/fname.ogg", "TitleFour", "1900-01-01", "ArtistFour", "AlbumTwo", "ArtistFour", 1, 5, AudioMedia);

    MediaStore store(":memory:", MS_READ_WRITE);
    store.insert(audio1);
    store.insert(audio2);
    store.insert(audio3);
    store.insert(audio4);

    // Query a track title
    vector<Album> albums = store.queryAlbums("TitleOne");
    ASSERT_EQ(albums.size(), 1);
    EXPECT_EQ(albums[0].getTitle(), "AlbumOne");
    EXPECT_EQ(albums[0].getArtist(), "Various Artists");

    // Query an album name
    albums = store.queryAlbums("AlbumTwo");
    ASSERT_EQ(albums.size(), 1);
    EXPECT_EQ(albums[0].getTitle(), "AlbumTwo");
    EXPECT_EQ(albums[0].getArtist(), "ArtistFour");

    // Query an artist name
    albums = store.queryAlbums("ArtistTwo");
    ASSERT_EQ(albums.size(), 1);
    EXPECT_EQ(albums[0].getTitle(), "AlbumOne");
    EXPECT_EQ(albums[0].getArtist(), "Various Artists");
}

TEST_F(MediaStoreTest, getAlbumSongs) {
    MediaFile audio1("/home/username/Music/track1.ogg", "TitleOne", "1900-01-01", "ArtistOne", "AlbumOne", "Various Artists", 1, 5, AudioMedia);
    MediaFile audio2("/home/username/Music/track2.ogg", "TitleTwo", "1900-01-01", "ArtistTwo", "AlbumOne", "Various Artists", 2, 5, AudioMedia);
    MediaFile audio3("/home/username/Music/track3.ogg", "TitleThree", "1900-01-01", "ArtistThree", "AlbumOne", "Various Artists", 3, 5, AudioMedia);

    MediaStore store(":memory:", MS_READ_WRITE);
    store.insert(audio1);
    store.insert(audio2);
    store.insert(audio3);

    vector<MediaFile> tracks = store.getAlbumSongs(
        Album("AlbumOne", "Various Artists"));
    ASSERT_EQ(tracks.size(), 3);
    EXPECT_EQ(tracks[0].getTitle(), "TitleOne");
    EXPECT_EQ(tracks[1].getTitle(), "TitleTwo");
    EXPECT_EQ(tracks[2].getTitle(), "TitleThree");
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
