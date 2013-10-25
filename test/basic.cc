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

using namespace std;

void init_test() {
    string base("basic");
    string fname = base + "-mediastore.db";
    unlink(fname.c_str());
    MediaStore store(base);
    SubtreeWatcher watcher(store);
}

int main() {
#ifdef NDEBUG
    fprintf(stderr, "NDEBUG defined, tests won't work.\n");
    return 1;
#else
    init_test();
    return 0;
#endif
}
