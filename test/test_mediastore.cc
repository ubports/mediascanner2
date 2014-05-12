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
#include <mediascanner/MediaFileBuilder.hh>
#include <mediascanner/Album.hh>
#include <mediascanner/MediaStore.hh>
#include <mediascanner/internal/utils.hh>

#include<stdexcept>
#include<cstdio>
#include<string>
#include <gtest/gtest.h>

using namespace std;
using namespace mediascanner;

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
TEST_F(MediaStoreTest, mediafile_uri) {
    MediaFile media("/path/to/file.ogg");
    EXPECT_EQ(media.getUri(), "file:///path/to/file.ogg");
}

TEST_F(MediaStoreTest, equality) {
    MediaFile audio1 = MediaFileBuilder("a")
        .setContentType("type")
        .setETag("etag")
        .setDate("1900")
        .setTitle("b")
        .setAuthor("c")
        .setAlbum("d")
        .setAlbumArtist("e")
        .setGenre("f")
        .setDiscNumber(0)
        .setTrackNumber(1)
        .setDuration(5)
        .setType(AudioMedia);
    MediaFile audio2 = MediaFileBuilder("aa")
        .setContentType("type")
        .setETag("etag")
        .setDate("1900")
        .setTitle("b")
        .setAuthor("c")
        .setAlbum("d")
        .setAlbumArtist("e")
        .setGenre("f")
        .setDiscNumber(0)
        .setTrackNumber(1)
        .setDuration(5)
        .setType(AudioMedia);

    MediaFile video1 = MediaFileBuilder("a")
        .setContentType("type")
        .setETag("etag")
        .setDate("1900")
        .setTitle("b")
        .setAuthor("c")
        .setAlbum("d")
        .setAlbumArtist("e")
        .setGenre("f")
        .setDiscNumber(0)
        .setTrackNumber(1)
        .setDuration(5)
        .setType(VideoMedia);
    MediaFile video2 = MediaFileBuilder("aa")
        .setContentType("type")
        .setETag("etag")
        .setDate("1900")
        .setTitle("b")
        .setAuthor("c")
        .setAlbum("d")
        .setAlbumArtist("e")
        .setGenre("f")
        .setDiscNumber(0)
        .setTrackNumber(1)
        .setDuration(5)
        .setType(VideoMedia);

    EXPECT_EQ(audio1, audio1);
    EXPECT_EQ(video1, video1);

    EXPECT_NE(audio1, audio2);
    EXPECT_NE(audio1, video1);
    EXPECT_NE(audio2, video1);
    EXPECT_NE(audio2, video2);
}

TEST_F(MediaStoreTest, lookup) {
    MediaFile audio = MediaFileBuilder("aaa")
        .setContentType("type")
        .setETag("etag")
        .setDate("1900-01-01")
        .setTitle("bbb bbb")
        .setAuthor("ccc")
        .setAlbum("ddd")
        .setAlbumArtist("eee")
        .setGenre("fff")
        .setDiscNumber(0)
        .setTrackNumber(3)
        .setDuration(5)
        .setType(AudioMedia);
    MediaStore store(":memory:", MS_READ_WRITE);
    store.insert(audio);

    EXPECT_EQ(store.lookup("aaa"), audio);
    EXPECT_THROW(store.lookup("not found"), std::runtime_error);
}

TEST_F(MediaStoreTest, roundtrip) {
    MediaFile audio = MediaFileBuilder("aaa")
        .setContentType("type")
        .setETag("etag")
        .setDate("1900-01-01")
        .setTitle("bbb bbb")
        .setAuthor("ccc")
        .setAlbum("ddd")
        .setAlbumArtist("eee")
        .setGenre("fff")
        .setDiscNumber(0)
        .setTrackNumber(3)
        .setDuration(5)
        .setType(AudioMedia);
    MediaFile video = MediaFileBuilder("aaa2")
        .setContentType("type")
        .setETag("etag")
        .setDate("2012-01-01")
        .setTitle("bbb bbb")
        .setAuthor("ccc")
        .setAlbum("ddd")
        .setAlbumArtist("eee")
        .setGenre("fff")
        .setDiscNumber(0)
        .setTrackNumber(0)
        .setDuration(5)
        .setType(VideoMedia);
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
    MediaFile audio = MediaFileBuilder("/path/foo.ogg")
        .setType(AudioMedia)
        .setTitle("title")
        .setAuthor("artist")
        .setAlbum("album")
        .setAlbumArtist("albumartist");
    MediaStore store(":memory:", MS_READ_WRITE);
    store.insert(audio);

    vector<MediaFile> result = store.query("album", AudioMedia);
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], audio);
 }

TEST_F(MediaStoreTest, query_by_artist) {
    MediaFile audio = MediaFileBuilder("/path/foo.ogg")
        .setType(AudioMedia)
        .setTitle("title")
        .setAuthor("artist")
        .setAlbum("album")
        .setAlbumArtist("albumartist");
    MediaStore store(":memory:", MS_READ_WRITE);
    store.insert(audio);

    vector<MediaFile> result = store.query("artist", AudioMedia);
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], audio);
 }

TEST_F(MediaStoreTest, query_ranking) {
    MediaFile audio1 = MediaFileBuilder("/path/foo1.ogg")
        .setType(AudioMedia)
        .setTitle("title")
        .setAuthor("artist")
        .setAlbum("album")
        .setAlbumArtist("albumartist");
    MediaFile audio2 = MediaFileBuilder("/path/foo2.ogg")
        .setType(AudioMedia)
        .setTitle("title aaa")
        .setAuthor("artist")
        .setAlbum("album")
        .setAlbumArtist("albumartist");
    MediaFile audio3 = MediaFileBuilder("/path/foo3.ogg")
        .setType(AudioMedia)
        .setTitle("title")
        .setAuthor("artist aaa")
        .setAlbum("album")
        .setAlbumArtist("albumartist");
    MediaFile audio4 = MediaFileBuilder("/path/foo4.ogg")
        .setType(AudioMedia)
        .setTitle("title")
        .setAuthor("artist")
        .setAlbum("album aaa")
        .setAlbumArtist("albumartist");
    MediaFile audio5 = MediaFileBuilder("/path/foo5.ogg")
        .setType(AudioMedia)
        .setTitle("title aaa")
        .setAuthor("artist aaa")
        .setAlbum("album aaa")
        .setAlbumArtist("albumartist");

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

TEST_F(MediaStoreTest, query_limit) {
    MediaFile audio1 = MediaFileBuilder("/path/foo5.ogg")
        .setType(AudioMedia)
        .setTitle("title aaa")
        .setAuthor("artist aaa")
        .setAlbum("album aaa")
        .setAlbumArtist("albumartist");
    MediaFile audio2 = MediaFileBuilder("/path/foo2.ogg")
        .setType(AudioMedia)
        .setTitle("title aaa")
        .setAuthor("artist")
        .setAlbum("album")
        .setAlbumArtist("albumartist");
    MediaFile audio3 = MediaFileBuilder("/path/foo4.ogg")
        .setType(AudioMedia)
        .setTitle("title")
        .setAuthor("artist")
        .setAlbum("album aaa")
        .setAlbumArtist("albumartist");

    MediaStore store(":memory:", MS_READ_WRITE);
    store.insert(audio1);
    store.insert(audio2);
    store.insert(audio3);

    vector<MediaFile> result = store.query("aaa", AudioMedia, 2);
    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], audio1); // Term appears in title, artist and album
    EXPECT_EQ(result[1], audio2); // title has highest weighting
}

TEST_F(MediaStoreTest, query_short) {
    MediaFile audio1 = MediaFileBuilder("/path/foo5.ogg")
        .setType(AudioMedia)
        .setTitle("title xyz")
        .setAuthor("artist")
        .setAlbum("album")
        .setAlbumArtist("albumartist");
    MediaFile audio2 = MediaFileBuilder("/path/foo2.ogg")
        .setType(AudioMedia)
        .setTitle("title xzy")
        .setAuthor("artist")
        .setAlbum("album")
        .setAlbumArtist("albumartist");

    MediaStore store(":memory:", MS_READ_WRITE);
    store.insert(audio1);
    store.insert(audio2);

    vector<MediaFile> result = store.query("x", AudioMedia);
    EXPECT_EQ(result.size(), 2);
    result = store.query("xy", AudioMedia);
    EXPECT_EQ(result.size(), 1);
}

TEST_F(MediaStoreTest, query_empty) {
    MediaFile audio1 = MediaFileBuilder("/path/foo5.ogg")
        .setType(AudioMedia)
        .setTitle("title aaa")
        .setAuthor("artist aaa")
        .setAlbum("album aaa")
        .setAlbumArtist("albumartist");
    MediaFile audio2 = MediaFileBuilder("/path/foo2.ogg")
        .setType(AudioMedia)
        .setTitle("title aaa")
        .setAuthor("artist")
        .setAlbum("album")
        .setAlbumArtist("albumartist");
    MediaFile audio3 = MediaFileBuilder("/path/foo4.ogg")
        .setType(AudioMedia)
        .setTitle("title")
        .setAuthor("artist")
        .setAlbum("album aaa")
        .setAlbumArtist("albumartist");

    MediaStore store(":memory:", MS_READ_WRITE);
    store.insert(audio1);
    store.insert(audio2);
    store.insert(audio3);

    // An empty query should return some results
    vector<MediaFile> result = store.query("", AudioMedia, 2);
    ASSERT_EQ(result.size(), 2);
}

TEST_F(MediaStoreTest, unmount) {
    MediaFile audio1 = MediaFileBuilder("/media/username/dir/fname.ogg")
        .setType(AudioMedia)
        .setTitle("bbb bbb");
    MediaFile audio2 = MediaFileBuilder("/home/username/Music/fname.ogg")
        .setType(AudioMedia)
        .setTitle("bbb bbb");
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
    MediaFile audio1 = MediaFileBuilder("/home/username/Music/track1.ogg")
        .setType(AudioMedia)
        .setTitle("TitleOne")
        .setAuthor("ArtistOne")
        .setAlbum("AlbumOne")
        .setAlbumArtist("Various Artists")
        .setDiscNumber(1)
        .setTrackNumber(1);
    MediaFile audio2 = MediaFileBuilder("/home/username/Music/track2.ogg")
        .setType(AudioMedia)
        .setTitle("TitleTwo")
        .setAuthor("ArtistTwo")
        .setAlbum("AlbumOne")
        .setAlbumArtist("Various Artists")
        .setDiscNumber(1)
        .setTrackNumber(2);
    MediaFile audio3 = MediaFileBuilder("/home/username/Music/track3.ogg")
        .setType(AudioMedia)
        .setTitle("TitleThree")
        .setAuthor("ArtistThree")
        .setAlbum("AlbumOne")
        .setAlbumArtist("Various Artists")
        .setDiscNumber(2)
        .setTrackNumber(1);
    MediaFile audio4 = MediaFileBuilder("/home/username/Music/fname.ogg")
        .setType(AudioMedia)
        .setTitle("TitleFour")
        .setAuthor("ArtistFour")
        .setAlbum("AlbumTwo")
        .setAlbumArtist("ArtistFour")
        .setTrackNumber(1);

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

TEST_F(MediaStoreTest, queryAlbums_limit) {
    MediaFile audio1 = MediaFileBuilder("/home/username/Music/track1.ogg")
        .setType(AudioMedia)
        .setTitle("TitleOne")
        .setAuthor("ArtistOne")
        .setAlbum("AlbumOne")
        .setAlbumArtist("Various Artists")
        .setDiscNumber(1)
        .setTrackNumber(1);
    MediaFile audio2 = MediaFileBuilder("/home/username/Music/track2.ogg")
        .setType(AudioMedia)
        .setTitle("TitleTwo")
        .setAuthor("ArtistTwo")
        .setAlbum("AlbumOne")
        .setAlbumArtist("Various Artists")
        .setDiscNumber(1)
        .setTrackNumber(2);
    MediaFile audio3 = MediaFileBuilder("/home/username/Music/track3.ogg")
        .setType(AudioMedia)
        .setTitle("TitleThree")
        .setAuthor("ArtistThree")
        .setAlbum("AlbumOne")
        .setAlbumArtist("Various Artists")
        .setDiscNumber(2)
        .setTrackNumber(1);
    MediaFile audio4 = MediaFileBuilder("/home/username/Music/fname.ogg")
        .setType(AudioMedia)
        .setTitle("TitleFour")
        .setAuthor("ArtistFour")
        .setAlbum("AlbumTwo")
        .setAlbumArtist("ArtistFour")
        .setTrackNumber(1);

    MediaStore store(":memory:", MS_READ_WRITE);
    store.insert(audio1);
    store.insert(audio2);
    store.insert(audio3);
    store.insert(audio4);

    vector<Album> albums = store.queryAlbums("Artist");
    EXPECT_EQ(2, albums.size());
    albums = store.queryAlbums("Artist", 1);
    EXPECT_EQ(1, albums.size());
}

TEST_F(MediaStoreTest, queryAlbums_empty) {
    MediaFile audio1 = MediaFileBuilder("/home/username/Music/track1.ogg")
        .setType(AudioMedia)
        .setTitle("TitleOne")
        .setAuthor("ArtistOne")
        .setAlbum("AlbumOne")
        .setAlbumArtist("Various Artists")
        .setDiscNumber(1)
        .setTrackNumber(1);
    MediaFile audio2 = MediaFileBuilder("/home/username/Music/track2.ogg")
        .setType(AudioMedia)
        .setTitle("TitleTwo")
        .setAuthor("ArtistTwo")
        .setAlbum("AlbumOne")
        .setAlbumArtist("Various Artists")
        .setDiscNumber(1)
        .setTrackNumber(2);
    MediaFile audio3 = MediaFileBuilder("/home/username/Music/track3.ogg")
        .setType(AudioMedia)
        .setTitle("TitleThree")
        .setAuthor("ArtistThree")
        .setAlbum("AlbumOne")
        .setAlbumArtist("Various Artists")
        .setDiscNumber(2)
        .setTrackNumber(1);
    MediaFile audio4 = MediaFileBuilder("/home/username/Music/fname.ogg")
        .setType(AudioMedia)
        .setTitle("TitleFour")
        .setAuthor("ArtistFour")
        .setAlbum("AlbumTwo")
        .setAlbumArtist("ArtistFour")
        .setTrackNumber(1);

    MediaStore store(":memory:", MS_READ_WRITE);
    store.insert(audio1);
    store.insert(audio2);
    store.insert(audio3);
    store.insert(audio4);

    vector<Album> albums = store.queryAlbums("");
    EXPECT_EQ(2, albums.size());
    albums = store.queryAlbums("", 1);
    EXPECT_EQ(1, albums.size());
}

TEST_F(MediaStoreTest, getAlbumSongs) {
    MediaFile audio1 = MediaFileBuilder("/home/username/Music/track1.ogg")
        .setType(AudioMedia)
        .setTitle("TitleOne")
        .setAuthor("ArtistOne")
        .setAlbum("AlbumOne")
        .setAlbumArtist("Various Artists")
        .setDiscNumber(1)
        .setTrackNumber(1);
    MediaFile audio2 = MediaFileBuilder("/home/username/Music/track2.ogg")
        .setType(AudioMedia)
        .setTitle("TitleTwo")
        .setAuthor("ArtistTwo")
        .setAlbum("AlbumOne")
        .setAlbumArtist("Various Artists")
        .setDiscNumber(1)
        .setTrackNumber(2);
    MediaFile audio3 = MediaFileBuilder("/home/username/Music/track3.ogg")
        .setType(AudioMedia)
        .setTitle("TitleThree")
        .setAuthor("ArtistThree")
        .setAlbum("AlbumOne")
        .setAlbumArtist("Various Artists")
        .setDiscNumber(2)
        .setTrackNumber(1);

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

TEST_F(MediaStoreTest, getETag) {
    MediaFile file = MediaFileBuilder("/path/file.ogg")
        .setETag("etag");

    MediaStore store(":memory:", MS_READ_WRITE);
    store.insert(file);

    EXPECT_EQ(store.getETag("/path/file.ogg"), "etag");
    EXPECT_EQ(store.getETag("/something-else.mp3"), "");
}

TEST_F(MediaStoreTest, listSongs) {
    MediaFile audio1 = MediaFileBuilder("/home/username/Music/track1.ogg")
        .setType(AudioMedia)
        .setTitle("TitleOne")
        .setAuthor("ArtistOne")
        .setAlbum("AlbumOne")
        .setTrackNumber(1);
    MediaFile audio2 = MediaFileBuilder("/home/username/Music/track2.ogg")
        .setType(AudioMedia)
        .setTitle("TitleTwo")
        .setAuthor("ArtistOne")
        .setAlbum("AlbumOne")
        .setTrackNumber(2);
    MediaFile audio3 = MediaFileBuilder("/home/username/Music/track3.ogg")
        .setType(AudioMedia)
        .setTitle("TitleThree")
        .setAuthor("ArtistOne")
        .setAlbum("AlbumTwo");
    MediaFile audio4 = MediaFileBuilder("/home/username/Music/track4.ogg")
        .setType(AudioMedia)
        .setTitle("TitleFour")
        .setAuthor("ArtistTwo")
        .setAlbum("AlbumThree");
    MediaFile audio5 = MediaFileBuilder("/home/username/Music/track5.ogg")
        .setType(AudioMedia)
        .setTitle("TitleFive")
        .setAuthor("ArtistOne")
        .setAlbum("AlbumFour")
        .setAlbumArtist("Various Artists")
        .setTrackNumber(1);
    MediaFile audio6 = MediaFileBuilder("/home/username/Music/track6.ogg")
        .setType(AudioMedia)
        .setTitle("TitleSix")
        .setAuthor("ArtistTwo")
        .setAlbum("AlbumFour")
        .setAlbumArtist("Various Artists")
        .setTrackNumber(2);

    MediaStore store(":memory:", MS_READ_WRITE);
    store.insert(audio1);
    store.insert(audio2);
    store.insert(audio3);
    store.insert(audio4);
    store.insert(audio5);
    store.insert(audio6);

    vector<MediaFile> tracks = store.listSongs();
    ASSERT_EQ(6, tracks.size());
    EXPECT_EQ("TitleOne", tracks[0].getTitle());

    // Apply a limit
    tracks = store.listSongs("", "", "", 4);
    EXPECT_EQ(4, tracks.size());

    // List songs by artist
    tracks = store.listSongs("ArtistOne");
    EXPECT_EQ(4, tracks.size());

    // List songs by album
    tracks = store.listSongs("", "AlbumOne");
    EXPECT_EQ(2, tracks.size());

    // List songs by album artist
    tracks = store.listSongs("", "", "Various Artists");
    EXPECT_EQ(2, tracks.size());

    // Combinations
    tracks = store.listSongs("ArtistOne", "AlbumOne", "");
    EXPECT_EQ(2, tracks.size());
    tracks = store.listSongs("", "AlbumOne", "ArtistOne");
    EXPECT_EQ(2, tracks.size());
    tracks = store.listSongs("ArtistOne", "AlbumOne", "ArtistOne");
    EXPECT_EQ(2, tracks.size());
    tracks = store.listSongs("ArtistOne", "", "ArtistOne");
    EXPECT_EQ(3, tracks.size());
}

TEST_F(MediaStoreTest, listAlbums) {
    MediaFile audio1 = MediaFileBuilder("/home/username/Music/track1.ogg")
        .setType(AudioMedia)
        .setTitle("TitleOne")
        .setAuthor("ArtistOne")
        .setAlbum("AlbumOne")
        .setTrackNumber(1);
    MediaFile audio2 = MediaFileBuilder("/home/username/Music/track3.ogg")
        .setType(AudioMedia)
        .setTitle("TitleThree")
        .setAuthor("ArtistOne")
        .setAlbum("AlbumTwo");
    MediaFile audio3 = MediaFileBuilder("/home/username/Music/track4.ogg")
        .setType(AudioMedia)
        .setTitle("TitleFour")
        .setAuthor("ArtistTwo")
        .setAlbum("AlbumThree");
    MediaFile audio4 = MediaFileBuilder("/home/username/Music/track5.ogg")
        .setType(AudioMedia)
        .setTitle("TitleFive")
        .setAuthor("ArtistOne")
        .setAlbum("AlbumFour")
        .setAlbumArtist("Various Artists")
        .setTrackNumber(1);
    MediaFile audio5 = MediaFileBuilder("/home/username/Music/track6.ogg")
        .setType(AudioMedia)
        .setTitle("TitleSix")
        .setAuthor("ArtistTwo")
        .setAlbum("AlbumFour")
        .setAlbumArtist("Various Artists")
        .setTrackNumber(2);

    MediaStore store(":memory:", MS_READ_WRITE);
    store.insert(audio1);
    store.insert(audio2);
    store.insert(audio3);
    store.insert(audio4);
    store.insert(audio5);

    vector<Album> albums = store.listAlbums();
    ASSERT_EQ(4, albums.size());
    EXPECT_EQ("AlbumOne", albums[0].getTitle());

    // test limit
    albums = store.listAlbums("", "", 2);
    EXPECT_EQ(2, albums.size());

    // Songs by artist
    albums = store.listAlbums("ArtistOne");
    EXPECT_EQ(3, albums.size());

    // Songs by album artist
    albums = store.listAlbums("", "ArtistOne");
    EXPECT_EQ(2, albums.size());

    // Combination
    albums = store.listAlbums("ArtistOne", "Various Artists");
    EXPECT_EQ(1, albums.size());
}

TEST_F(MediaStoreTest, listArtists) {
    MediaFile audio1 = MediaFileBuilder("/home/username/Music/track1.ogg")
        .setType(AudioMedia)
        .setTitle("TitleOne")
        .setAuthor("ArtistOne")
        .setAlbum("AlbumOne")
        .setTrackNumber(1);
    MediaFile audio2 = MediaFileBuilder("/home/username/Music/track3.ogg")
        .setType(AudioMedia)
        .setTitle("TitleThree")
        .setAuthor("ArtistOne")
        .setAlbum("AlbumTwo");
    MediaFile audio3 = MediaFileBuilder("/home/username/Music/track4.ogg")
        .setType(AudioMedia)
        .setTitle("TitleFour")
        .setAuthor("ArtistTwo")
        .setAlbum("AlbumThree");
    MediaFile audio4 = MediaFileBuilder("/home/username/Music/track5.ogg")
        .setType(AudioMedia)
        .setTitle("TitleFive")
        .setAuthor("ArtistOne")
        .setAlbum("AlbumFour")
        .setAlbumArtist("Various Artists")
        .setTrackNumber(1);
    MediaFile audio5 = MediaFileBuilder("/home/username/Music/track6.ogg")
        .setType(AudioMedia)
        .setTitle("TitleSix")
        .setAuthor("ArtistTwo")
        .setAlbum("AlbumFour")
        .setAlbumArtist("Various Artists")
        .setTrackNumber(2);

    MediaStore store(":memory:", MS_READ_WRITE);
    store.insert(audio1);
    store.insert(audio2);
    store.insert(audio3);
    store.insert(audio4);
    store.insert(audio5);

    vector<string> artists = store.listArtists(false);
    ASSERT_EQ(2, artists.size());
    EXPECT_EQ("ArtistOne", artists[0]);
    EXPECT_EQ("ArtistTwo", artists[1]);

    // Test limit clause
    artists = store.listArtists(false, 1);
    EXPECT_EQ(1, artists.size());

    // List "album artists"
    artists = store.listArtists(true);
    ASSERT_EQ(3, artists.size());
    EXPECT_EQ("ArtistOne", artists[0]);
    EXPECT_EQ("ArtistTwo", artists[1]);
    EXPECT_EQ("Various Artists", artists[2]);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
