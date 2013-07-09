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
#include <dirent.h>
#include <sys/stat.h>
#include<cstdio>
using namespace std;

Scanner::Scanner() {

}

Scanner::~Scanner() {

}

vector<string> Scanner::scanFiles(const std::string &root) {
    vector<string> result;
    DIR* dir = opendir(root.c_str());
    printf("In subdir %s\n", root.c_str());
    if(!dir) {
        return result;
    }
    struct dirent* curloc;
    while( (curloc = readdir(dir)) ) {
        struct stat statbuf;
        string fname = curloc->d_name;
        if(fname == "." || fname == "..")
            continue;
        string fullpath = root + "/" + fname;
        stat(fullpath.c_str(), &statbuf);
        if(S_ISREG(statbuf.st_mode)) {
            result.push_back(fullpath);
        } else if(S_ISDIR(statbuf.st_mode)) {
            vector<string> subdir = scanFiles(fullpath);
            result.insert(result.end(), subdir.begin(), subdir.end());
        }
    }
    return result;
}
