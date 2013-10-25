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

#include "MediaFile.hh"
#include "FileTypeDetector.hh"
#include "metadataextractor.hh"
#include <stdexcept>

using namespace std;

MediaFile::MediaFile(std::string filename) : filename(filename) {
    FileTypeDetector d;
    type = d.detect(filename);
    if(type == UnknownMedia) {
        throw runtime_error("Tried to create an invalid media type.");
    }
    getMetadata(filename, title, author, album, duration);
}

MediaFile::MediaFile(std::string filename, std::string title, std::string author, std::string album,
        int duration, MediaType type) :
    filename(filename), title(title), author(author), album(album), duration(duration), type(type) {

}

const std::string& MediaFile::getFileName() const noexcept {
    return filename;
}
const std::string& MediaFile::getTitle() const noexcept {
    return title;
}

const std::string& MediaFile::getAuthor() const noexcept {
    return author;
}

const std::string& MediaFile::getAlbum() const noexcept {
    return album;
}

int MediaFile::getDuration() const noexcept {
    return duration;
}

MediaType MediaFile::getType() const noexcept {
    return type;
}
