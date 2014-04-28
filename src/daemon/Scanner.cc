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

#include "MetadataExtractor.hh"
#include "Scanner.hh"
#include "util.h"
#include <dirent.h>
#include <sys/stat.h>
#include<cstdio>
#include<memory>

using namespace std;

namespace mediascanner {

struct FileGenerator {
    FileGenerator();

    vector<string> dirs;
    unique_ptr<struct dirent, void(*)(void*)> entry;
    unique_ptr<DIR, int(*)(DIR*)> dir;
    MediaType type;
    MetadataExtractor *extractor;
};

FileGenerator::FileGenerator() : entry(nullptr, free), dir(nullptr, closedir), type(AllMedia), extractor(nullptr) {

}

Scanner::Scanner() {
}

Scanner::~Scanner() {
}

vector<DetectedFile> Scanner::scanFiles(MetadataExtractor *extractor, const std::string &root, const MediaType type) {
    unique_ptr<FileGenerator> g(generator(extractor, root, type));
    vector<DetectedFile> result;
    while(true) {
        try {
            result.push_back(next(g.get()));
        } catch(const StopIteration &e) {
            return result;
        }
    }
    throw std::runtime_error("Unreachable code.");
}

FileGenerator* Scanner::generator(MetadataExtractor *extractor, const std::string &root, const MediaType type) {
    FileGenerator *g = new FileGenerator();
    g->dirs.push_back(root);
    g->type = type;
    g->extractor = extractor;
    return g;
}

DetectedFile Scanner::next(FileGenerator* g) {
    string curdir;
    while(!g->dir) {
        if(g->dirs.empty()) {
            throw StopIteration();
        }
        curdir = g->dirs.back();
        g->dirs.pop_back();
        unique_ptr<DIR, int(*)(DIR*)> tmp(opendir(curdir.c_str()), closedir);
        g->dir = move(tmp);
        printf("In subdir %s\n", curdir.c_str());
    }
    if(is_rootlike(curdir)) {
        fprintf(stderr, "Directory %s looks like a top level root directory, skipping it (%s).\n",
                curdir.c_str(), __PRETTY_FUNCTION__);
    } else {
        unique_ptr<struct dirent, void(*)(void*)> tmpentry((dirent*)malloc(sizeof(dirent) + NAME_MAX),
                free);
        g->entry = std::move(tmpentry);
        struct dirent *de;
        while(readdir_r(g->dir.get(), g->entry.get(), &de) == 0 && de ) {
            struct stat statbuf;
            string fname = g->entry.get()->d_name;
            if(fname[0] == '.') // Ignore hidden files and dirs.
                continue;
            string fullpath = curdir + "/" + fname;
            lstat(fullpath.c_str(), &statbuf);
            if(S_ISREG(statbuf.st_mode)) {
                try {
                    DetectedFile d = g->extractor->detect(fullpath);
                    if (g->type == AllMedia || d.type == g->type) {
                        return d;
                    }
                } catch (const exception &e) {
                    /* Ignore non-media files */
                }
            } else if(S_ISDIR(statbuf.st_mode)) {
                g->dirs.push_back(fullpath);
            }
        }
    }
    // Nothing left in this directory so on to the next.
    g->dir.reset();
    return next(g);
}

}
