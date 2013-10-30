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

#include<MediaFile.hh>
#include<MediaStore.hh>
#include<SubtreeWatcher.hh>
#include<cassert>
#include<cstdio>
#include<string>
#include<unistd.h>
#include<sys/stat.h>
#include<gst/gst.h>
#include<Scanner.hh>

using namespace std;

void init_test() {
    string base("basic");
    string fname = base + "-mediastore.db";
    unlink(fname.c_str());
    MediaStore store(fname, MS_READ_WRITE);
    SubtreeWatcher watcher(store);
}

void clear_dir(const string &subdir) {
    string cmd = "rm -rf " + subdir;
    system(cmd.c_str()); // Because I like to live dangerously, that's why.
}


void copy_file(const string &src, const string &dst) {
    FILE* f = fopen(src.c_str(), "r");
    assert(f);
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);

    char* buf = new char[size];

    fseek(f, 0, SEEK_SET);
    assert(fread(buf, 1, size, f) == size);
    fclose(f);

    f = fopen(dst.c_str(), "w");
    assert(f);
    assert(fwrite(buf, 1, size, f) == size);
    fclose(f);
    delete[] buf;
}

void index_test() {
    string base("index");
    string dbname = base + "-mediastore.db";
    string subdir = getenv("TEST_DIR");
    subdir += "/testdir";
    string testfile = getenv("SOURCE_DIR");
    testfile += "/test/testfile.ogg";
    string outfile = subdir + "/testfile.ogg";
    unlink(dbname.c_str());
    clear_dir(subdir);
    assert(mkdir(subdir.c_str(), S_IRUSR | S_IWUSR | S_IXUSR) >= 0);
    MediaStore store(dbname, MS_READ_WRITE);
    SubtreeWatcher watcher(store);
    watcher.addDir(subdir);
    assert(store.size() == 0);

    copy_file(testfile, outfile);
    watcher.pumpEvents();
    assert(store.size() == 1);
    assert(unlink(outfile.c_str()) == 0);
    watcher.pumpEvents();
    assert(store.size() == 0);
}

void subdir_test() {
    string base("index");
    string dbname = base + "-mediastore.db";
    string testdir = getenv("TEST_DIR");
    testdir += "/testdir";
    string subdir = testdir + "/subdir";
    string testfile = getenv("SOURCE_DIR");
    testfile += "/test/testfile.ogg";
    string outfile = subdir + "/testfile.ogg";
    unlink(dbname.c_str());
    clear_dir(testdir);
    assert(mkdir(testdir.c_str(), S_IRUSR | S_IWUSR | S_IXUSR) >= 0);
    MediaStore store(dbname, MS_READ_WRITE);
    SubtreeWatcher watcher(store);
    watcher.addDir(testdir);
    assert(store.size() == 0);

    assert(mkdir(subdir.c_str(), S_IRUSR | S_IWUSR | S_IXUSR) >= 0);
    watcher.pumpEvents();
    copy_file(testfile, outfile);
    watcher.pumpEvents();
    assert(store.size() == 1);
    assert(unlink(outfile.c_str()) == 0);
    watcher.pumpEvents();
    assert(store.size() == 0);
}

// FIXME move this somewhere in the implementation.
void scanFiles(MediaStore &store, const string &subdir, const MediaType type) {
    Scanner s;
    vector<string> files = s.scanFiles(subdir, type);
    for(auto &i : files) {
        store.insert(MediaFile(i));
    }
}

void scan_test() {
    string base("index");
    string dbname = base + "-mediastore.db";
    string testdir = getenv("TEST_DIR");
    testdir += "/testdir";
    string testfile = getenv("SOURCE_DIR");
    testfile += "/test/testfile.ogg";
    string outfile = testdir + "/testfile.ogg";
    unlink(dbname.c_str());
    clear_dir(testdir);
    assert(mkdir(testdir.c_str(), S_IRUSR | S_IWUSR | S_IXUSR) >= 0);
    copy_file(testfile, outfile);
    MediaStore *store = new MediaStore(dbname, MS_READ_WRITE);
    scanFiles(*store, testdir, AudioMedia);
    assert(store->size() == 1);

    delete store;
    unlink(outfile.c_str());
    store = new MediaStore(dbname, MS_READ_WRITE);
    store->pruneDeleted();
    assert(store->size() == 0);
    delete store;

}

void equality_test() {
    MediaFile audio1("a", "b", "c", "d", 5, AudioMedia);
    MediaFile audio2("aa", "b", "c", "d", 5, AudioMedia);

    MediaFile video1("a", "b", "c", "d", 5, VideoMedia);
    MediaFile video2("aa", "b", "c", "d", 5, VideoMedia);

    assert(audio1 == audio1);
    assert(video1 == video1);

    assert(audio1 != audio2);
    assert(audio1 != video1);
    assert(audio2 != video1);
    assert(audio2 != video2);
}

void roundtrip_test() {
    MediaFile audio("aaa", "bbb bbb", "ccc", "ddd", 5, AudioMedia);
    MediaFile video("aaa2", "bbb bbb", "ccc", "ddd", 5, VideoMedia);
    string base("roundtrip");
    string dbname = base + "-mediastore.db";
    unlink(dbname.c_str());
    MediaStore store(dbname, MS_READ_WRITE);
    store.insert(audio);
    store.insert(video);
    vector<MediaFile> result = store.query("bbb", AudioMedia);
    assert(result.size() == 1);
    assert(result[0] == audio);
    result = store.query("bbb", VideoMedia);
    assert(result.size() == 1);
    assert(result[0] == video);
}

void unmount_test() {
    MediaFile audio1("/media/username/dir/fname.ogg", "bbb bbb", "ccc", "ddd", 5, AudioMedia);
    MediaFile audio2("/home/username/Music/fname.ogg", "bbb bbb", "ccc", "ddd", 5, AudioMedia);
    string base("unmount");
    string dbname = base + "-mediastore.db";
    unlink(dbname.c_str());
    MediaStore store(dbname, MS_READ_WRITE);
    store.insert(audio1);
    store.insert(audio2);
    vector<MediaFile> result = store.query("bbb", AudioMedia);
    assert(result.size() == 2);

    store.archiveItems("/media/username");
    result = store.query("bbb", AudioMedia);
    assert(result.size() == 1);
    assert(result[0] == audio2);

    store.restoreItems("/media/username");
    result = store.query("bbb", AudioMedia);
    assert(result.size() == 2);
}

int main(int argc, char **argv) {
    gst_init (&argc, &argv);
#ifdef NDEBUG
    fprintf(stderr, "NDEBUG defined, tests won't work.\n");
    return 1;
#else
    init_test();
    index_test();
    subdir_test();
    scan_test();
    equality_test();
    roundtrip_test();
    unmount_test();
    return 0;
#endif
}
