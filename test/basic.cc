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
#include <daemon/MetadataExtractor.hh>
#include <daemon/SubtreeWatcher.hh>
#include <daemon/FileTypeDetector.hh>
#include <daemon/Scanner.hh>

#include<stdexcept>
#include<cstdio>
#include<string>
#include<unistd.h>
#include<sys/stat.h>
#include<gst/gst.h>
#include <gtest/gtest.h>

using namespace std;

class ScanTest : public ::testing::Test {
 protected:
  ScanTest() {
  }

  virtual ~ScanTest() {
  }

  virtual void SetUp() {
  }

  virtual void TearDown() {
  }
};

TEST_F(ScanTest, init) {
    string base("basic");
    string fname = base + "-mediastore.db";
    unlink(fname.c_str());
    MediaStore store(fname, MS_READ_WRITE);
    MetadataExtractor extractor;
    SubtreeWatcher watcher(store, extractor);
}

void clear_dir(const string &subdir) {
    string cmd = "rm -rf " + subdir;
    ASSERT_EQ(system(cmd.c_str()), 0); // Because I like to live dangerously, that's why.
}


void copy_file(const string &src, const string &dst) {
    FILE* f = fopen(src.c_str(), "r");
    ASSERT_TRUE(f);
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);

    char* buf = new char[size];

    fseek(f, 0, SEEK_SET);
    ASSERT_EQ(fread(buf, 1, size, f), size);
    fclose(f);

    f = fopen(dst.c_str(), "w");
    ASSERT_TRUE(f);
    ASSERT_EQ(fwrite(buf, 1, size, f), size);
    fclose(f);
    delete[] buf;
}

TEST_F(ScanTest, index) {
    string dbname("index-mediastore.db");
    string subdir = getenv("TEST_DIR");
    subdir += "/testdir";
    string testfile = getenv("SOURCE_DIR");
    testfile += "/test/testfile.ogg";
    string outfile = subdir + "/testfile.ogg";
    unlink(dbname.c_str());
    clear_dir(subdir);
    ASSERT_GE(mkdir(subdir.c_str(), S_IRUSR | S_IWUSR | S_IXUSR), 0);
    MediaStore store(dbname, MS_READ_WRITE);
    MetadataExtractor extractor;
    SubtreeWatcher watcher(store, extractor);
    watcher.addDir(subdir);
    ASSERT_EQ(store.size(), 0);

    copy_file(testfile, outfile);
    watcher.pumpEvents();
    ASSERT_EQ(store.size(), 1);
    ASSERT_EQ(unlink(outfile.c_str()), 0);
    watcher.pumpEvents();
    ASSERT_EQ(store.size(), 0);
}

TEST_F(ScanTest, extract) {
    MetadataExtractor e;
    string testfile = getenv("SOURCE_DIR");
    testfile += "/test/testfile.ogg";
    MediaFile file = e.extract(testfile);

    ASSERT_EQ(file.getTitle(), "track1");
    ASSERT_EQ(file.getAuthor(), "artist1");
    ASSERT_EQ(file.getAlbum(), "album1");
    ASSERT_EQ(file.getDate(), "2013");
    ASSERT_EQ(file.getTrackNumber(), 1);
    ASSERT_EQ(file.getDuration(), 5);

    string nomediafile = getenv("SOURCE_DIR");
    nomediafile += "/CMakeLists.txt";
    ASSERT_THROW(e.extract(nomediafile), runtime_error);
}

TEST_F(ScanTest, subdir) {
    string dbname("subdir-mediastore.db");
    string testdir = getenv("TEST_DIR");
    testdir += "/testdir";
    string subdir = testdir + "/subdir";
    string testfile = getenv("SOURCE_DIR");
    testfile += "/test/testfile.ogg";
    string outfile = subdir + "/testfile.ogg";
    unlink(dbname.c_str());
    clear_dir(testdir);
    ASSERT_GE(mkdir(testdir.c_str(), S_IRUSR | S_IWUSR | S_IXUSR), 0);
    MediaStore store(dbname, MS_READ_WRITE);
    MetadataExtractor extractor;
    SubtreeWatcher watcher(store, extractor);
    ASSERT_EQ(watcher.directoryCount(), 0);
    watcher.addDir(testdir);
    ASSERT_EQ(watcher.directoryCount(), 1);
    ASSERT_EQ(store.size(), 0);

    ASSERT_GE(mkdir(subdir.c_str(), S_IRUSR | S_IWUSR | S_IXUSR), 0);
    watcher.pumpEvents();
    ASSERT_EQ(watcher.directoryCount(), 2);
    copy_file(testfile, outfile);
    watcher.pumpEvents();
    ASSERT_EQ(store.size(), 1);
    ASSERT_EQ(unlink(outfile.c_str()), 0);
    watcher.pumpEvents();
    ASSERT_EQ(store.size(), 0);
    ASSERT_EQ(rmdir(subdir.c_str()), 0);
    watcher.pumpEvents();
    ASSERT_EQ(watcher.directoryCount(), 1);
}

// FIXME move this somewhere in the implementation.
void scanFiles(MediaStore &store, const string &subdir, const MediaType type) {
    Scanner s;
    MetadataExtractor extractor;
    vector<string> files = s.scanFiles(subdir, type);
    for(auto &i : files) {
        store.insert(extractor.extract(i));
    }
}

TEST_F(ScanTest, scan) {
    string dbname("scan-mediastore.db");
    string testdir = getenv("TEST_DIR");
    testdir += "/testdir";
    string testfile = getenv("SOURCE_DIR");
    testfile += "/test/testfile.ogg";
    string outfile = testdir + "/testfile.ogg";
    unlink(dbname.c_str());
    clear_dir(testdir);
    ASSERT_GE(mkdir(testdir.c_str(), S_IRUSR | S_IWUSR | S_IXUSR), 0);
    copy_file(testfile, outfile);
    MediaStore *store = new MediaStore(dbname, MS_READ_WRITE);
    scanFiles(*store, testdir, AudioMedia);
    ASSERT_EQ(store->size(), 1);

    delete store;
    unlink(outfile.c_str());
    store = new MediaStore(dbname, MS_READ_WRITE);
    store->pruneDeleted();
    ASSERT_EQ(store->size(), 0);
    delete store;

}

TEST_F(ScanTest, equality) {
    MediaFile audio1("a", "1900", "b", "c", "d", "e", 1, 5, AudioMedia);
    MediaFile audio2("aa", "1900", "b", "c", "d", "e", 1, 5, AudioMedia);

    MediaFile video1("a", "b", "1900", "c", "d", "e", 0, 5, VideoMedia);
    MediaFile video2("aa", "b", "1900", "c", "d", "e", 0, 5, VideoMedia);

    ASSERT_EQ(audio1, audio1);
    ASSERT_EQ(video1, video1);

    ASSERT_NE(audio1, audio2);
    ASSERT_NE(audio1, video1);
    ASSERT_NE(audio2, video1);
    ASSERT_NE(audio2, video2);
}

TEST_F(ScanTest, roundtrip) {
    MediaFile audio("aaa", "bbb bbb", "1900-01-01", "ccc", "ddd", "eee", 3, 5, AudioMedia);
    MediaFile video("aaa2", "bbb bbb", "2012-01-01", "ccc", "ddd", "eee", 0, 5, VideoMedia);
    string dbname("roundtrip-mediastore.db");
    unlink(dbname.c_str());
    MediaStore store(dbname, MS_READ_WRITE);
    store.insert(audio);
    store.insert(video);
    vector<MediaFile> result = store.query("bbb", AudioMedia);
    ASSERT_EQ(result.size(), 1);
    ASSERT_EQ(result[0], audio);
    result = store.query("bbb", VideoMedia);
    ASSERT_EQ(result.size(), 1);
    ASSERT_EQ(result[0], video);
}

TEST_F(ScanTest, unmount) {
    MediaFile audio1("/media/username/dir/fname.ogg", "bbb bbb", "2000-01-01", "ccc", "ddd", "eee", 1, 5, AudioMedia);
    MediaFile audio2("/home/username/Music/fname.ogg", "bbb bbb", "1900-01-01", "ccc", "ddd", "eee", 42, 5, AudioMedia);
    string dbname("unmount-mediastore.db");
    unlink(dbname.c_str());
    MediaStore store(dbname, MS_READ_WRITE);
    store.insert(audio1);
    store.insert(audio2);
    vector<MediaFile> result = store.query("bbb", AudioMedia);
    ASSERT_EQ(result.size(), 2);

    store.archiveItems("/media/username");
    result = store.query("bbb", AudioMedia);
    ASSERT_EQ(result.size(), 1);
    ASSERT_EQ(result[0], audio2);

    store.restoreItems("/media/username");
    result = store.query("bbb", AudioMedia);
    ASSERT_EQ(result.size(), 2);
}

TEST_F(ScanTest, detector) {
    FileTypeDetector d;
    string testfile = getenv("SOURCE_DIR");
    testfile += "/test/testfile.ogg";
    string nomediafile = getenv("SOURCE_DIR");
    nomediafile += "/CMakeLists.txt";
    ASSERT_EQ(d.detect(testfile), AudioMedia);
    ASSERT_EQ(d.detect("/a/non/existing/file"), UnknownMedia);
    ASSERT_EQ(d.detect(nomediafile), UnknownMedia);
}

TEST_F(ScanTest, utils) {
    string source("_a.b(c)[d]{e}f.mp3");
    string correct = {" a b c  d  e f"};
    string result = filenameToTitle(source);
    ASSERT_EQ(correct, result);

    string unquoted(R"(It's a living.)");
    string quoted(R"('It''s a living.')");
    ASSERT_EQ(sqlQuote(unquoted), quoted);
}

TEST_F(ScanTest, queryAlbums) {
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

TEST_F(ScanTest, getAlbumSongs) {
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
    gst_init (&argc, &argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
