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
    MediaFile(std::string filename, std::string title, std::string author, std::string album,
            int duration, MediaType type);
    MediaFile() = delete;

    const std::string& getFileName() const noexcept;
    const std::string& getTitle() const noexcept;
    const std::string& getAuthor() const noexcept;
    const std::string& getAlbum() const noexcept;
    int getDuration() const noexcept;
    MediaType getType() const noexcept;
    bool operator==(const MediaFile &other) const;
    bool operator!=(const MediaFile &other) const;

private:
    std::string filename;
    std::string title;
    std::string author;
    std::string album;
    int duration; // In seconds.
    MediaType type;
};

#endif

