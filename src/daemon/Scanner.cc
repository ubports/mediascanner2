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

#include "Scanner.hh"
#include "../extractor/DetectedFile.hh"
#include "../extractor/MetadataExtractor.hh"
#include "../mediascanner/internal/utils.hh"
#include <dirent.h>
#include <sys/stat.h>
#include<cstdio>
#include<cstdlib>
#include<memory>
#include<cassert>

using namespace std;

namespace mediascanner {

struct Scanner::Private {
    Private(MetadataExtractor *extractor_, const std::string &root, const MediaType type_);

    string curdir;
    vector<string> dirs;
    unique_ptr<struct dirent, void(*)(void*)> entry;
    unique_ptr<DIR, int(*)(DIR*)> dir;
    MediaType type;
    MetadataExtractor *extractor;
    struct dirent *de;
};

Scanner::Private::Private(MetadataExtractor *extractor, const std::string &root, const MediaType type) :
        entry((dirent*)malloc(sizeof(dirent) + NAME_MAX + 1), free),
        dir(nullptr, closedir),
        type(type),
        extractor(extractor),
        de(nullptr)
{
    dirs.push_back(root);
}

Scanner::Scanner(MetadataExtractor *extractor, const std::string &root, const MediaType type) :
    p(new Scanner::Private(extractor, root, type)) {
}

Scanner::~Scanner() {
    delete p;
}


DetectedFile Scanner::next() {
begin:
    while(!p->dir) {
        if(p->dirs.empty()) {
            throw StopIteration();
        }
        p->curdir = p->dirs.back();
        p->dirs.pop_back();
        unique_ptr<DIR, int(*)(DIR*)> tmp(opendir(p->curdir.c_str()), closedir);
        p->dir = move(tmp);
        if(is_rootlike(p->curdir)) {
            fprintf(stderr, "Directory %s looks like a top level root directory, skipping it (%s).\n",
                    p->curdir.c_str(), __PRETTY_FUNCTION__);
            p->dir.reset();
            continue;
        }
        if(has_scanblock(p->curdir)) {
            fprintf(stderr, "Directory %s has a scan block file, skipping it.\n",
                    p->curdir.c_str());
            p->dir.reset();
            continue;
        }
        printf("In subdir %s\n", p->curdir.c_str());
    }

    while(readdir_r(p->dir.get(), p->entry.get(), &p->de) == 0 && p->de ) {
        struct stat statbuf;
        string fname = p->entry.get()->d_name;
        if(fname[0] == '.') // Ignore hidden files and dirs.
            continue;
        string fullpath = p->curdir + "/" + fname;
        lstat(fullpath.c_str(), &statbuf);
        if(S_ISREG(statbuf.st_mode)) {
            try {
                DetectedFile d = p->extractor->detect(fullpath);
                if (p->type == AllMedia || d.type == p->type) {
                    return d;
                }
            } catch (const exception &e) {
                /* Ignore non-media files */
            }
        } else if(S_ISDIR(statbuf.st_mode)) {
            p->dirs.push_back(fullpath);
        }
    }

    // Nothing left in this directory so on to the next.
    assert(!p->de);
    p->dir.reset();
    // This should be just return next(s) but we can't guarantee
    // that GCC can optimize away the tail recursion so we do this
    // instead. Using goto instead of wrapping the whole function body in
    // a while(true) to avoid the extra indentation.
    goto begin;
}

}
