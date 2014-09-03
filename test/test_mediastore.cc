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
#include <mediascanner/Filter.hh>
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
    MediaFile media = MediaFileBuilder("/path/to/file.ogg");
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
        .setDuration(5)
        .setWidth(41)
        .setHeight(42)
        .setType(VideoMedia);
    MediaFile video2 = MediaFileBuilder("aa")
        .setContentType("type")
        .setETag("etag")
        .setDate("1900")
        .setTitle("b")
        .setDuration(5)
        .setWidth(43)
        .setHeight(44)
        .setType(VideoMedia);

    MediaFile image1 = MediaFileBuilder("a")
        .setContentType("type")
        .setETag("etag")
        .setDate("1900")
        .setTitle("b")
        .setWidth(640)
        .setHeight(480)
        .setLatitude(20.0)
        .setLongitude(30.0)
        .setType(ImageMedia);
    MediaFile image2 = MediaFileBuilder("aa")
        .setContentType("type")
        .setETag("etag")
        .setDate("1900")
        .setTitle("b")
        .setWidth(480)
        .setHeight(640)
        .setLatitude(-20.0)
        .setLongitude(-30.0)
        .setType(ImageMedia);

    EXPECT_EQ(audio1, audio1);
    EXPECT_EQ(video1, video1);
    EXPECT_EQ(image1, image1);

    EXPECT_NE(audio1, audio2);
    EXPECT_NE(video1, video2);
    EXPECT_NE(image1, image2);

    EXPECT_NE(audio1, video1);
    EXPECT_NE(audio1, image1);
    EXPECT_NE(video1, image1);
}

TEST_F(MediaStoreTest, lookup) {
    MediaFile audio = MediaFileBuilder("/aaa")
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

    EXPECT_EQ(store.lookup("/aaa"), audio);
    EXPECT_THROW(store.lookup("not found"), std::runtime_error);
}

TEST_F(MediaStoreTest, roundtrip) {
    MediaFile audio = MediaFileBuilder("/aaa")
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
        .setHasThumbnail(true)
        .setType(AudioMedia);
    MediaFile video = MediaFileBuilder("/aaa2")
        .setContentType("type")
        .setETag("etag")
        .setDate("2012-01-01")
        .setTitle("bbb bbb")
        .setDuration(5)
        .setWidth(1280)
        .setHeight(720)
        .setType(VideoMedia);
    MediaFile image = MediaFileBuilder("/aaa3")
        .setContentType("type")
        .setETag("etag")
        .setDate("2012-01-01")
        .setTitle("bbb bbb")
        .setWidth(480)
        .setHeight(640)
        .setLatitude(20.0)
        .setLongitude(-30.0)
        .setType(ImageMedia);
    MediaStore store(":memory:", MS_READ_WRITE);
    store.insert(audio);
    store.insert(video);
    store.insert(image);

    Filter filter;
    vector<MediaFile> result = store.query("bbb", AudioMedia, filter);
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], audio);

    result = store.query("bbb", VideoMedia, filter);
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], video);

    result = store.query("bbb", ImageMedia, filter);
    ASSERT_EQ(1, result.size());
    EXPECT_EQ(image, result[0]);
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

    Filter filter;
    vector<MediaFile> result = store.query("album", AudioMedia, filter);
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

    Filter filter;
    vector<MediaFile> result = store.query("artist", AudioMedia, filter);
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

    Filter filter;
    vector<MediaFile> result = store.query("aaa", AudioMedia, filter);
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

    Filter filter;
    filter.setLimit(2);
    vector<MediaFile> result = store.query("aaa", AudioMedia, filter);
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

    Filter filter;
    vector<MediaFile> result = store.query("x", AudioMedia, filter);
    EXPECT_EQ(result.size(), 2);
    result = store.query("xy", AudioMedia, filter);
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
    Filter filter;
    filter.setLimit(2);
    vector<MediaFile> result = store.query("", AudioMedia, filter);
    ASSERT_EQ(result.size(), 2);
}

TEST_F(MediaStoreTest, query_order) {
    MediaFile audio1 = MediaFileBuilder("/path/foo1.ogg")
        .setType(AudioMedia)
        .setTitle("foo")
        .setDate("2010-01-01")
        .setAuthor("artist")
        .setAlbum("album");
    MediaFile audio2 = MediaFileBuilder("/path/foo2.ogg")
        .setType(AudioMedia)
        .setTitle("foo foo")
        .setDate("2010-01-03")
        .setAuthor("artist")
        .setAlbum("album");
    MediaFile audio3 = MediaFileBuilder("/path/foo3.ogg")
        .setType(AudioMedia)
        .setTitle("foo foo foo")
        .setDate("2010-01-02")
        .setAuthor("artist")
        .setAlbum("album");

    MediaStore store(":memory:", MS_READ_WRITE);
    store.insert(audio1);
    store.insert(audio2);
    store.insert(audio3);

    Filter filter;
    // Default sort order is by rank
    vector<MediaFile> result = store.query("foo", AudioMedia, filter);
    ASSERT_EQ(3, result.size());
    EXPECT_EQ("/path/foo3.ogg", result[0].getFileName());
    EXPECT_EQ("/path/foo2.ogg", result[1].getFileName());
    EXPECT_EQ("/path/foo1.ogg", result[2].getFileName());

    // Sorting by rank (same as default)
    filter.setOrder(MediaOrder::Rank);
    result = store.query("foo", AudioMedia, filter);
    ASSERT_EQ(3, result.size());
    EXPECT_EQ("/path/foo3.ogg", result[0].getFileName());
    EXPECT_EQ("/path/foo2.ogg", result[1].getFileName());
    EXPECT_EQ("/path/foo1.ogg", result[2].getFileName());

    // Sorting by rank, reversed
    filter.setReverse(true);
    result = store.query("foo", AudioMedia, filter);
    ASSERT_EQ(3, result.size());
    EXPECT_EQ("/path/foo1.ogg", result[0].getFileName());
    EXPECT_EQ("/path/foo2.ogg", result[1].getFileName());
    EXPECT_EQ("/path/foo3.ogg", result[2].getFileName());

    // Sorting by title
    filter.setReverse(false);
    filter.setOrder(MediaOrder::Title);
    result = store.query("foo", AudioMedia, filter);
    ASSERT_EQ(3, result.size());
    EXPECT_EQ("/path/foo1.ogg", result[0].getFileName());
    EXPECT_EQ("/path/foo2.ogg", result[1].getFileName());
    EXPECT_EQ("/path/foo3.ogg", result[2].getFileName());

    // Sorting by title, reversed
    filter.setReverse(true);
    result = store.query("foo", AudioMedia, filter);
    ASSERT_EQ(3, result.size());
    EXPECT_EQ("/path/foo3.ogg", result[0].getFileName());
    EXPECT_EQ("/path/foo2.ogg", result[1].getFileName());
    EXPECT_EQ("/path/foo1.ogg", result[2].getFileName());

    // Sorting by date
    filter.setReverse(false);
    filter.setOrder(MediaOrder::Date);
    result = store.query("foo", AudioMedia, filter);
    ASSERT_EQ(3, result.size());
    EXPECT_EQ("/path/foo1.ogg", result[0].getFileName());
    EXPECT_EQ("/path/foo3.ogg", result[1].getFileName());
    EXPECT_EQ("/path/foo2.ogg", result[2].getFileName());

    // Sorting by date, reversed
    filter.setReverse(true);
    result = store.query("foo", AudioMedia, filter);
    ASSERT_EQ(3, result.size());
    EXPECT_EQ("/path/foo2.ogg", result[0].getFileName());
    EXPECT_EQ("/path/foo3.ogg", result[1].getFileName());
    EXPECT_EQ("/path/foo1.ogg", result[2].getFileName());
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
    Filter filter;
    vector<MediaFile> result = store.query("bbb", AudioMedia, filter);
    ASSERT_EQ(result.size(), 2);

    store.archiveItems("/media/username");
    result = store.query("bbb", AudioMedia, filter);
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], audio2);

    store.restoreItems("/media/username");
    result = store.query("bbb", AudioMedia, filter);
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
        .setDate("2000-01-01")
        .setDiscNumber(1)
        .setTrackNumber(1)
        .setGenre("GenreOne");
    MediaFile audio2 = MediaFileBuilder("/home/username/Music/track2.ogg")
        .setType(AudioMedia)
        .setTitle("TitleTwo")
        .setAuthor("ArtistTwo")
        .setAlbum("AlbumOne")
        .setAlbumArtist("Various Artists")
        .setDate("2000-01-01")
        .setDiscNumber(1)
        .setTrackNumber(2)
        .setGenre("GenreOne");
    MediaFile audio3 = MediaFileBuilder("/home/username/Music/track3.ogg")
        .setType(AudioMedia)
        .setTitle("TitleThree")
        .setAuthor("ArtistThree")
        .setAlbum("AlbumOne")
        .setAlbumArtist("Various Artists")
        .setDate("2000-01-01")
        .setDiscNumber(2)
        .setTrackNumber(1)
        .setGenre("GenreOne");
    MediaFile audio4 = MediaFileBuilder("/home/username/Music/fname.ogg")
        .setType(AudioMedia)
        .setTitle("TitleFour")
        .setAuthor("ArtistFour")
        .setAlbum("AlbumTwo")
        .setAlbumArtist("ArtistFour")
        .setDate("2014-06-01")
        .setTrackNumber(1)
        .setGenre("GenreTwo")
        .setHasThumbnail(true);

    MediaStore store(":memory:", MS_READ_WRITE);
    store.insert(audio1);
    store.insert(audio2);
    store.insert(audio3);
    store.insert(audio4);

    // Query a track title
    Filter filter;
    vector<Album> albums = store.queryAlbums("TitleOne", filter);
    ASSERT_EQ(albums.size(), 1);
    EXPECT_EQ(albums[0].getTitle(), "AlbumOne");
    EXPECT_EQ(albums[0].getArtist(), "Various Artists");
    EXPECT_EQ(albums[0].getDate(), "2000-01-01");
    EXPECT_EQ(albums[0].getGenre(), "GenreOne");
    EXPECT_EQ(albums[0].getArtUri(), "image://albumart/artist=Various%20Artists&album=AlbumOne");

    // Query an album name
    albums = store.queryAlbums("AlbumTwo", filter);
    ASSERT_EQ(albums.size(), 1);
    EXPECT_EQ(albums[0].getTitle(), "AlbumTwo");
    EXPECT_EQ(albums[0].getArtist(), "ArtistFour");
    EXPECT_EQ(albums[0].getDate(), "2014-06-01");
    EXPECT_EQ(albums[0].getGenre(), "GenreTwo");
    EXPECT_EQ(albums[0].getArtUri(), "image://thumbnailer/file:///home/username/Music/fname.ogg");

    // Query an artist name
    albums = store.queryAlbums("ArtistTwo", filter);
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

    Filter filter;
    vector<Album> albums = store.queryAlbums("Artist", filter);
    EXPECT_EQ(2, albums.size());
    filter.setLimit(1);
    albums = store.queryAlbums("Artist", filter);
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

    Filter filter;
    vector<Album> albums = store.queryAlbums("", filter);
    EXPECT_EQ(2, albums.size());
    filter.setLimit(1);
    albums = store.queryAlbums("", filter);
    EXPECT_EQ(1, albums.size());
}

TEST_F(MediaStoreTest, queryAlbums_order) {
    MediaFile audio1 = MediaFileBuilder("/path/foo1.ogg")
        .setType(AudioMedia)
        .setTitle("title")
        .setDate("2010-01-01")
        .setAuthor("artist")
        .setAlbum("foo");
    MediaFile audio2 = MediaFileBuilder("/path/foo2.ogg")
        .setType(AudioMedia)
        .setTitle("title")
        .setDate("2010-01-03")
        .setAuthor("artist")
        .setAlbum("foo foo");
    MediaFile audio3 = MediaFileBuilder("/path/foo3.ogg")
        .setType(AudioMedia)
        .setTitle("title")
        .setDate("2010-01-02")
        .setAuthor("artist")
        .setAlbum("foo foo foo");

    MediaStore store(":memory:", MS_READ_WRITE);
    store.insert(audio1);
    store.insert(audio2);
    store.insert(audio3);

    // Default sort
    Filter filter;
    vector<Album> albums = store.queryAlbums("foo", filter);
    ASSERT_EQ(3, albums.size());
    EXPECT_EQ("foo", albums[0].getTitle());
    EXPECT_EQ("foo foo", albums[1].getTitle());
    EXPECT_EQ("foo foo foo", albums[2].getTitle());

    // Sort by title (same as default)
    filter.setOrder(MediaOrder::Title);
    albums = store.queryAlbums("foo", filter);
    ASSERT_EQ(3, albums.size());
    EXPECT_EQ("foo", albums[0].getTitle());
    EXPECT_EQ("foo foo", albums[1].getTitle());
    EXPECT_EQ("foo foo foo", albums[2].getTitle());

    // Sort by title, reversed
    filter.setReverse(true);
    albums = store.queryAlbums("foo", filter);
    ASSERT_EQ(3, albums.size());
    EXPECT_EQ("foo foo foo", albums[0].getTitle());
    EXPECT_EQ("foo foo", albums[1].getTitle());
    EXPECT_EQ("foo", albums[2].getTitle());

    // Other orders are not supported
    filter.setOrder(MediaOrder::Rank);
    EXPECT_THROW(store.queryAlbums("foo", filter), std::runtime_error);
    filter.setOrder(MediaOrder::Date);
    EXPECT_THROW(store.queryAlbums("foo", filter), std::runtime_error);
}

TEST_F(MediaStoreTest, queryArtists) {
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
    Filter filter;
    vector<string> artists = store.queryArtists("TitleOne", filter);
    ASSERT_EQ(artists.size(), 1);
    EXPECT_EQ(artists[0], "ArtistOne");

    // Query an album name
    artists = store.queryArtists("AlbumTwo", filter);
    ASSERT_EQ(artists.size(), 1);
    EXPECT_EQ(artists[0], "ArtistFour");

    // Query an artist name
    artists = store.queryArtists("ArtistTwo", filter);
    ASSERT_EQ(artists.size(), 1);
    EXPECT_EQ(artists[0], "ArtistTwo");
}

TEST_F(MediaStoreTest, queryArtists_limit) {
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

    MediaStore store(":memory:", MS_READ_WRITE);
    store.insert(audio1);
    store.insert(audio2);

    Filter filter;
    vector<string> artists = store.queryArtists("Artist", filter);
    EXPECT_EQ(2, artists.size());
    filter.setLimit(1);
    artists = store.queryArtists("Artist", filter);
    EXPECT_EQ(1, artists.size());
}

TEST_F(MediaStoreTest, queryArtists_empty) {
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

    MediaStore store(":memory:", MS_READ_WRITE);
    store.insert(audio1);
    store.insert(audio2);

    Filter filter;
    vector<string> artists = store.queryArtists("", filter);
    EXPECT_EQ(2, artists.size());
    filter.setLimit(1);
    artists = store.queryArtists("", filter);
    EXPECT_EQ(1, artists.size());
}

TEST_F(MediaStoreTest, queryArtists_order) {
    MediaFile audio1 = MediaFileBuilder("/path/foo1.ogg")
        .setType(AudioMedia)
        .setTitle("title")
        .setDate("2010-01-01")
        .setAuthor("foo")
        .setAlbum("album");
    MediaFile audio2 = MediaFileBuilder("/path/foo2.ogg")
        .setType(AudioMedia)
        .setTitle("title")
        .setDate("2010-01-03")
        .setAuthor("foo foo")
        .setAlbum("album");
    MediaFile audio3 = MediaFileBuilder("/path/foo3.ogg")
        .setType(AudioMedia)
        .setTitle("title")
        .setDate("2010-01-02")
        .setAuthor("foo foo foo")
        .setAlbum("album");

    MediaStore store(":memory:", MS_READ_WRITE);
    store.insert(audio1);
    store.insert(audio2);
    store.insert(audio3);

    // Default sort
    Filter filter;
    vector<std::string> artists = store.queryArtists("foo", filter);
    ASSERT_EQ(3, artists.size());
    EXPECT_EQ("foo", artists[0]);
    EXPECT_EQ("foo foo", artists[1]);
    EXPECT_EQ("foo foo foo", artists[2]);

    // Sort by title (same as default)
    filter.setOrder(MediaOrder::Title);
    artists = store.queryArtists("foo", filter);
    ASSERT_EQ(3, artists.size());
    EXPECT_EQ("foo", artists[0]);
    EXPECT_EQ("foo foo", artists[1]);
    EXPECT_EQ("foo foo foo", artists[2]);

    // Sort by title, reversed
    filter.setReverse(true);
    artists = store.queryArtists("foo", filter);
    ASSERT_EQ(3, artists.size());
    EXPECT_EQ("foo foo foo", artists[0]);
    EXPECT_EQ("foo foo", artists[1]);
    EXPECT_EQ("foo", artists[2]);

    // Other orders are not supported
    filter.setOrder(MediaOrder::Rank);
    EXPECT_THROW(store.queryArtists("foo", filter), std::runtime_error);
    filter.setOrder(MediaOrder::Date);
    EXPECT_THROW(store.queryArtists("foo", filter), std::runtime_error);
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
        .setETag("etag")
        .setType(AudioMedia);

    MediaStore store(":memory:", MS_READ_WRITE);
    store.insert(file);

    EXPECT_EQ(store.getETag("/path/file.ogg"), "etag");
    EXPECT_EQ(store.getETag("/something-else.mp3"), "");
}


TEST_F(MediaStoreTest, constraints) {
    MediaFile file = MediaFileBuilder("no_slash_at_beginning.ogg")
        .setETag("etag")
        .setType(AudioMedia);
    MediaFile file2 = MediaFileBuilder("/invalid_type.ogg")
        .setETag("etag");

    MediaStore store(":memory:", MS_READ_WRITE);
    ASSERT_THROW(store.insert(file), std::runtime_error);
    ASSERT_THROW(store.insert(file2), std::runtime_error);
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

    Filter filter;
    vector<MediaFile> tracks = store.listSongs(filter);
    ASSERT_EQ(6, tracks.size());
    EXPECT_EQ("TitleOne", tracks[0].getTitle());

    // Apply a limit
    filter.setLimit(4);
    tracks = store.listSongs(filter);
    EXPECT_EQ(4, tracks.size());
    filter.setLimit(-1);

    // List songs by artist
    filter.setArtist("ArtistOne");
    tracks = store.listSongs(filter);
    EXPECT_EQ(4, tracks.size());

    // List songs by album
    filter.clear();
    filter.setAlbum("AlbumOne");
    tracks = store.listSongs(filter);
    EXPECT_EQ(2, tracks.size());

    // List songs by album artist
    filter.clear();
    filter.setAlbumArtist("Various Artists");
    tracks = store.listSongs(filter);
    EXPECT_EQ(2, tracks.size());

    // Combinations
    filter.clear();
    filter.setArtist("ArtistOne");
    filter.setAlbum("AlbumOne");
    tracks = store.listSongs(filter);
    EXPECT_EQ(2, tracks.size());

    filter.clear();
    filter.setAlbum("AlbumOne");
    filter.setAlbumArtist("ArtistOne");
    tracks = store.listSongs(filter);
    EXPECT_EQ(2, tracks.size());

    filter.clear();
    filter.setArtist("ArtistOne");
    filter.setAlbum("AlbumOne");
    filter.setAlbumArtist("ArtistOne");
    tracks = store.listSongs(filter);
    EXPECT_EQ(2, tracks.size());

    filter.clear();
    filter.setArtist("ArtistOne");
    filter.setAlbumArtist("ArtistOne");
    tracks = store.listSongs(filter);
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

    Filter filter;
    vector<Album> albums = store.listAlbums(filter);
    ASSERT_EQ(4, albums.size());
    EXPECT_EQ("AlbumOne", albums[0].getTitle());

    // test limit
    filter.setLimit(2);
    albums = store.listAlbums(filter);
    EXPECT_EQ(2, albums.size());
    filter.setLimit(-1);

    // Songs by artist
    filter.setArtist("ArtistOne");
    albums = store.listAlbums(filter);
    EXPECT_EQ(3, albums.size());

    // Songs by album artist
    filter.clear();
    filter.setAlbumArtist("ArtistOne");
    albums = store.listAlbums(filter);
    EXPECT_EQ(2, albums.size());

    // Combination
    filter.clear();
    filter.setArtist("ArtistOne");
    filter.setAlbumArtist("Various Artists");
    albums = store.listAlbums(filter);
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

    Filter filter;
    vector<string> artists = store.listArtists(filter);
    ASSERT_EQ(2, artists.size());
    EXPECT_EQ("ArtistOne", artists[0]);
    EXPECT_EQ("ArtistTwo", artists[1]);

    // Test limit clause
    filter.setLimit(1);
    artists = store.listArtists(filter);
    EXPECT_EQ(1, artists.size());
    filter.setLimit(-1);

    // List "album artists"
    artists = store.listAlbumArtists(filter);
    ASSERT_EQ(3, artists.size());
    EXPECT_EQ("ArtistOne", artists[0]);
    EXPECT_EQ("ArtistTwo", artists[1]);
    EXPECT_EQ("Various Artists", artists[2]);
}

TEST_F(MediaStoreTest, brokenFiles) {
    MediaStore store(":memory:", MS_READ_WRITE);
    std::string file = "/foo/bar/baz.mp3";
    std::string other_file = "/foo/bar/abc.mp3";
    std::string broken_etag = "123";
    std::string ok_etag = "124";

    ASSERT_FALSE(store.is_broken_file(file, broken_etag));
    ASSERT_FALSE(store.is_broken_file(file, ok_etag));
    ASSERT_FALSE(store.is_broken_file(other_file, ok_etag));
    ASSERT_FALSE(store.is_broken_file(other_file, broken_etag));

    store.insert_broken_file(file, broken_etag);
    ASSERT_TRUE(store.is_broken_file(file, broken_etag));
    ASSERT_FALSE(store.is_broken_file(file, ok_etag));
    ASSERT_FALSE(store.is_broken_file(other_file, ok_etag));
    ASSERT_FALSE(store.is_broken_file(other_file, broken_etag));

    store.remove_broken_file(file);
    ASSERT_FALSE(store.is_broken_file(file, broken_etag));
    ASSERT_FALSE(store.is_broken_file(file, ok_etag));
    ASSERT_FALSE(store.is_broken_file(other_file, ok_etag));
    ASSERT_FALSE(store.is_broken_file(other_file, broken_etag));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
