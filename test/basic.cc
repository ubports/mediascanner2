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
#include <mediascanner/MediaStore.hh>
#include <daemon/MetadataExtractor.hh>
#include <daemon/SubtreeWatcher.hh>
#include <daemon/FileTypeDetector.hh>
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
    string subdir = TEST_DIR "/testdir";
    string testfile = SOURCE_DIR "/media/testfile.ogg";
    string outfile = subdir + "/testfile.ogg";
    clear_dir(subdir);
    ASSERT_GE(mkdir(subdir.c_str(), S_IRUSR | S_IWUSR | S_IXUSR), 0);
    MediaStore store(":memory:", MS_READ_WRITE);
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

TEST_F(ScanTest, subdir) {
    string testdir = TEST_DIR "/testdir";
    string subdir = testdir + "/subdir";
    string testfile = SOURCE_DIR "/media/testfile.ogg";
    string outfile = subdir + "/testfile.ogg";
    clear_dir(testdir);
    ASSERT_GE(mkdir(testdir.c_str(), S_IRUSR | S_IWUSR | S_IXUSR), 0);
    MediaStore store(":memory:", MS_READ_WRITE);
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

int main(int argc, char **argv) {
    gst_init (&argc, &argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
