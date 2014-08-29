/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * Authors:
 *    Jussi Pakkanen <jussi.pakkanen@canonical.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include"internal/utils.hh"

#include<vector>
#include<stdexcept>
#include<glib.h>
#include<sys/stat.h>
#include<cstring>
#include<cerrno>

namespace mediascanner {

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
            result = filename.substr(0, suffix_dot);
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

std::string getUri(const std::string &filename) {
    GError *error = nullptr;
    char *uristr = g_filename_to_uri(filename.c_str(), "", &error);
    if (error) {
        std::string msg("Could not build URI: ");
        msg += error->message;
        g_error_free(error);
        throw std::runtime_error(msg);
    }

    std::string uri(uristr);
    g_free(uristr);
    return uri;
}

using namespace std;

static bool dir_exists(const string &path) {
    struct stat statbuf;
    if(stat(path.c_str(), &statbuf) < 0) {
        if(errno != ENOENT) {
            printf("Error while trying to determine state of dir %s: %s\n", path.c_str(), strerror(errno));
        }
        return false;
    }
    return S_ISDIR(statbuf.st_mode) ;
}

static bool file_exists(const string &path) {
    struct stat statbuf;
    if(stat(path.c_str(), &statbuf) < 0) {
        if(errno != ENOENT) {
            printf("Error while trying to determine state of file %s: %s\n", path.c_str(), strerror(errno));
        }
        return false;
    }
    return S_ISREG(statbuf.st_mode) ;
}

bool is_rootlike(const string &path) {
    string s1 = path + "/usr";
    string s2 = path + "/var";
    string s3 = path + "/bin";
    string s4 = path + "/Program Files";
    return (dir_exists(s1) && dir_exists(s2) && dir_exists(s3)) || dir_exists(s4);
}

bool is_optical_disc(const string &path) {
    string dvd1 = path + "/AUDIO_TS";
    string dvd2 = path + "/VIDEO_TS";
    string bluray = path + "/BDMV";
    return (dir_exists(dvd1) && dir_exists(dvd2)) || dir_exists(bluray);
}

bool has_scanblock(const std::string &path) {
    return file_exists(path + "/.nomedia");
}

static string uri_escape(const string &unescaped) {
    char *result = g_uri_escape_string(unescaped.c_str(), NULL, FALSE);
    string escaped(result);
    g_free(result);
    return escaped;
}

string make_album_art_uri(const string &artist, const string &album) {
    string result = "image://albumart/artist=";
    result += uri_escape(artist);
    result += "&album=";
    result += uri_escape(album);
    return result;
}

std::string make_thumbnail_uri(const std::string &uri) {
    return string("image://thumbnailer/") + uri;
}

}

