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

#ifndef MEDIAFILEBUILDER_H_
#define MEDIAFILEBUILDER_H_

#include"scannercore.hh"
#include<string>

namespace mediascanner {

class MediaFile;
struct MediaFilePrivate;

/**
 * This is a helper class to build MediaFiles. Since we want MediaFiles
 * to be immutable and always valid, the user needs to always list
 * all variables in the constructor. This is cumbersome so this class
 * allows you to gather them one by one and then finally construct
 * a fully valid MediaFile.
 */

class MediaFileBuilder final {
    friend class MediaFile;
public:
    explicit MediaFileBuilder(const std::string &filename);
    MediaFileBuilder(const MediaFile &mf);
    MediaFileBuilder(const MediaFileBuilder &) = delete;
    MediaFileBuilder& operator=(MediaFileBuilder &) = delete;
    ~MediaFileBuilder();

    MediaFile build() const;

    MediaFileBuilder &setType(MediaType t);
    MediaFileBuilder &setETag(const std::string &e);
    MediaFileBuilder &setContentType(const std::string &c);
    MediaFileBuilder &setTitle(const std::string &t);
    MediaFileBuilder &setDate(const std::string &d);
    MediaFileBuilder &setAuthor(const std::string &a);
    MediaFileBuilder &setAlbum(const std::string &a);
    MediaFileBuilder &setAlbumArtist(const std::string &a);
    MediaFileBuilder &setGenre(const std::string &g);
    MediaFileBuilder &setDiscNumber(int n);
    MediaFileBuilder &setTrackNumber(int n);
    MediaFileBuilder &setDuration(int d);
    MediaFileBuilder &setWidth(int w);
    MediaFileBuilder &setHeight(int h);
    MediaFileBuilder &setLatitude(double l);
    MediaFileBuilder &setLongitude(double l);
    MediaFileBuilder &setHasThumbnail(bool t);

private:
    MediaFilePrivate *p;
};

}

#endif
