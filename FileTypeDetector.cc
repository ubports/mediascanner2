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

#include"FileTypeDetector.hh"
#include<cstdio>
bool endsWith(const std::string &text, const std::string &end) {
    auto loc = text.rfind(end);
    if(loc == std::string::npos)
        return false;
    return true; // Should check that the match was at the end of the string.
}

MediaType FileTypeDetector::detect(const std::string &fname) {
    std::string mine = fname;
    for(size_t i=0; i<mine.size(); i++) {
        mine[i] = tolower(mine[i]);
    }
    if(endsWith(mine, ".mp3") || endsWith(mine, ".ogg") ||
       endsWith(mine, ".flac") || endsWith(mine, ".wav")) {
        return AudioMedia;
    }
    if(endsWith(mine, ".avi") || endsWith(mine, ".mkv") ||
       endsWith(mine, ".mov") || endsWith(mine, ".mpg")) {
        return VideoMedia;
    }
    return UnknownMedia;
}
