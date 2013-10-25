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

#include<MediaStore.hh>
#include<SubtreeWatcher.hh>
#include<cassert>
#include<cstdio>
#include<string>
#include<unistd.h>
#include<sys/stat.h>

using namespace std;

void init_test() {
    string base("basic");
    string fname = base + "-mediastore.db";
    unlink(fname.c_str());
    MediaStore store(base);
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
    string outfile = subdir + "testfile.ogg";
    unlink(dbname.c_str());
    clear_dir(subdir);
    assert(mkdir(subdir.c_str(), S_IRUSR | S_IWUSR | S_IXUSR) >= 0);
    MediaStore store(base);
    SubtreeWatcher watcher(store);
    watcher.addDir(subdir);

    copy_file(testfile, outfile);
    watcher.pumpEvents();
    // assert that file is in
    assert(unlink(outfile.c_str()) == 0);
    watcher.pumpEvents();
    // assert that file is out
}

int main() {
#ifdef NDEBUG
    fprintf(stderr, "NDEBUG defined, tests won't work.\n");
    return 1;
#else
    init_test();
    index_test();
    return 0;
#endif
}
