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

#include"utils.hh"

#include<vector>

std::string sqlQuote(const std::string &input) {
    std::vector<char> out;
    out.reserve(input.size() + 2);
    const char quote = '\'';
    out.push_back(quote);
    for(size_t i=0; i<input.size(); i++) {
        char x = input[i];
        if(x == quote)
            out.push_back(quote);
        out.push_back(x);
    }
    out.push_back(quote);
    out.push_back('\0');
    return std::string(&out[0]);
}

// Convert filename into something that full text search
// will be able to find. That is, separate words with spaces.
#include<cstdio>
std::string filenameToTitle(const std::string &filename) {
    auto fname_start = filename.rfind('/');
    auto suffix_dot = filename.rfind('.');
    std::string result;
    if(fname_start == std::string::npos) {
        if(suffix_dot == std::string::npos) {
            result = filename;
        } else {
            result = filename.substr(0, suffix_dot-1);
        }
    } else {
        if(suffix_dot == std::string::npos) {
            result = filename.substr(fname_start+1, filename.size());
        } else {
            result = filename.substr(fname_start+1, suffix_dot-fname_start-1);
        }
    }
    for(size_t i=0; i<result.size(); i++) {
        auto c = result[i];
        if(c == '.' || c == '_' || c == '(' || c == ')' ||
           c == '[' || c == ']' || c == '{' || c == '}' ||
           c == '\\') {
            result[i] = ' ';
        }
    }
    return result;
}
