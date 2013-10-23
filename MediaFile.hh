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

#ifndef MEDIAFILE_HH
#define MEDIAFILE_HH

#include"scannercore.hh"
#include<string>

class MediaFile {
public:
    MediaFile(std::string filename);
    MediaFile(std::string filename, std::string title, std::string author, std::string album,
            MediaType type);
    MediaFile() = delete;

    std::string getFileName() const;
    std::string getTitle() const;
    std::string getAuthor() const;
    std::string getAlbum() const;
    int getDuration() const;
    MediaType getType() const;

private:
    std::string filename;
    std::string title;
    std::string author;
    std::string album;
    MediaType type;
};

#endif

