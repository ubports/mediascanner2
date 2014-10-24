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
#include <mediascanner/MediaStore.hh>
#include <daemon/InvalidationSender.hh>
#include <daemon/MetadataExtractor.hh>
#include <daemon/SubtreeWatcher.hh>
#include <daemon/Scanner.hh>

#include "test_config.h"

#include<stdexcept>
#include<cstdio>
#include<string>
#include<unistd.h>
#include<sys/stat.h>
#include<gst/gst.h>
#include <gtest/gtest.h>

using namespace std;
using namespace mediascanner;

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
    MediaStore store(":memory:", MS_READ_WRITE);
    MetadataExtractor extractor;
    InvalidationSender invalidator;
    SubtreeWatcher watcher(store, extractor, invalidator);
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

void iterate_main_loop() {
    while (g_main_context_iteration(nullptr, FALSE)) {
    }
}

TEST_F(ScanTest, index) {
    string subdir = TEST_DIR "/testdir";
    string testfile = SOURCE_DIR "/media/testfile.ogg";
    string outfile = subdir + "/testfile.ogg";
    clear_dir(subdir);
    ASSERT_GE(mkdir(subdir.c_str(), S_IRUSR | S_IWUSR | S_IXUSR), 0);
    MediaStore store(":memory:", MS_READ_WRITE);
    MetadataExtractor extractor;
    InvalidationSender invalidator;
    SubtreeWatcher watcher(store, extractor, invalidator);
    watcher.addDir(subdir);
    ASSERT_EQ(store.size(), 0);

    copy_file(testfile, outfile);
    iterate_main_loop();
    ASSERT_EQ(store.size(), 1);
    ASSERT_EQ(unlink(outfile.c_str()), 0);
    iterate_main_loop();
    ASSERT_EQ(store.size(), 0);
}

TEST_F(ScanTest, subdir) {
    string testdir = TEST_DIR "/testdir";
    string subdir = testdir + "/subdir";
    string testfile = SOURCE_DIR "/media/testfile.ogg";
    string outfile = subdir + "/testfile.ogg";
    clear_dir(testdir);
    ASSERT_GE(mkdir(testdir.c_str(), S_IRUSR | S_IWUSR | S_IXUSR), 0);
    MediaStore store(":memory:", MS_READ_WRITE);
    MetadataExtractor extractor;
    InvalidationSender invalidator;
    SubtreeWatcher watcher(store, extractor, invalidator);
    ASSERT_EQ(watcher.directoryCount(), 0);
    watcher.addDir(testdir);
    ASSERT_EQ(watcher.directoryCount(), 1);
    ASSERT_EQ(store.size(), 0);

    ASSERT_GE(mkdir(subdir.c_str(), S_IRUSR | S_IWUSR | S_IXUSR), 0);
    iterate_main_loop();
    ASSERT_EQ(watcher.directoryCount(), 2);
    copy_file(testfile, outfile);
    iterate_main_loop();
    ASSERT_EQ(store.size(), 1);
    ASSERT_EQ(unlink(outfile.c_str()), 0);
    iterate_main_loop();
    ASSERT_EQ(store.size(), 0);
    ASSERT_EQ(rmdir(subdir.c_str()), 0);
    iterate_main_loop();
    ASSERT_EQ(watcher.directoryCount(), 1);
}

// FIXME move this somewhere in the implementation.
void scanFiles(MediaStore &store, const string &subdir, const MediaType type) {
    MetadataExtractor extractor;
    Scanner s(&extractor, subdir, type);
    try {
        while(true) {
            auto d  = s.next();
            // If the file is unchanged or known bad, do fallback.
            if (store.is_broken_file(d.filename, d.etag)) {
                fprintf(stderr, "Using fallback data for unscannable file %s.\n", d.filename.c_str());
                store.insert(extractor.fallback_extract(d));
                continue;
            }
            if(d.etag == store.getETag(d.filename)) {
                continue;
            }
            store.insert_broken_file(d.filename, d.etag);
            store.insert(extractor.extract(d));
            // If the above line crashes, then brokenness of this file
            // persists.
        }
    } catch(const StopIteration &e) {
    }
}

TEST_F(ScanTest, scan) {
    string dbname("scan-mediastore.db");
    string testdir = TEST_DIR "/testdir";
    string testfile = SOURCE_DIR "/media/testfile.ogg";
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

TEST_F(ScanTest, scan_skips_unchanged_files) {
    string testdir = TEST_DIR "/testdir";
    string testfile = SOURCE_DIR "/media/testfile.ogg";
    string outfile = testdir + "/testfile.ogg";
    clear_dir(testdir);
    ASSERT_GE(mkdir(testdir.c_str(), S_IRUSR | S_IWUSR | S_IXUSR), 0);
    copy_file(testfile, outfile);

    MediaStore store(":memory:", MS_READ_WRITE);
    scanFiles(store, testdir, AudioMedia);
    ASSERT_EQ(store.size(), 1);

    /* Modify the metadata for this file in the database */
    MediaFile media = store.lookup(outfile);
    EXPECT_NE(media.getETag(), "");
    EXPECT_EQ(media.getTitle(), "track1");
    MediaFileBuilder mfb(media);
    mfb.setTitle("something else");
    store.insert(mfb.build());

    /* Scan again, and note that the data hasn't been updated */
    scanFiles(store, testdir, AudioMedia);
    media = store.lookup(outfile);
    EXPECT_EQ(media.getTitle(), "something else");

    /* Now change the stored etag, to trigger an update */
    MediaFileBuilder mfb2(media);
    mfb2.setETag("old-etag");
    store.insert(mfb2.build());
    scanFiles(store, testdir, AudioMedia);
    media = store.lookup(outfile);
    EXPECT_EQ(media.getTitle(), "track1");
}

TEST(Mediascanner, root_skip) {
    MetadataExtractor e;
    string root(SOURCE_DIR);
    Scanner s(&e, root, AudioMedia);
    s.next();
    ASSERT_THROW(s.next(), StopIteration);
}

TEST(Mediascanner, scan_files_found_in_new_dir) {
    string testdir = TEST_DIR "/testdir";
    string subdir = testdir + "/subdir";
    string testfile = SOURCE_DIR "/media/testfile.ogg";
    string outfile = subdir + "/testfile.ogg";
    clear_dir(testdir);
    ASSERT_GE(mkdir(testdir.c_str(), S_IRUSR | S_IWUSR | S_IXUSR), 0);

    MediaStore store(":memory:", MS_READ_WRITE);
    MetadataExtractor extractor;
    InvalidationSender invalidator;
    SubtreeWatcher watcher(store, extractor, invalidator);
    watcher.addDir(testdir);
    ASSERT_EQ(watcher.directoryCount(), 1);
    ASSERT_EQ(store.size(), 0);

    // Create a new directory and a file inside that directory before
    // the watcher has a chance to set up an inotify watch.
    ASSERT_GE(mkdir(subdir.c_str(), S_IRUSR | S_IWUSR | S_IXUSR), 0);
    copy_file(testfile, outfile);
    iterate_main_loop();
    ASSERT_EQ(watcher.directoryCount(), 2);
    ASSERT_EQ(store.size(), 1);
}

int main(int argc, char **argv) {
    gst_init (&argc, &argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
