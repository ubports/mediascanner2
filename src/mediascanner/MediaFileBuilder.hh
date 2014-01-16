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

#ifndef MEDIAFILEBUILDER_H_
#define MEDIAFILEBUILDER_H_

#include"scannercore.hh"
#include<string>

namespace mediascanner {

class MediaFile;

/**
 * This is a helper class to build MediaFiles. Since we want MediaFiles
 * to be immutable and always valid, the user needs to always list
 * all variables in the constructor. This is cumbersome so this class
 * allows you to gather them one by one and then finally construct
 * a fully valid MediaFile.
 *
 * If you try to assign the same property twice, an exception is thrown.
 * This means that MediaFileBuilders are meant to build only one
 * MediaFile. To build a new one create a new MediaFileBuilder. This is to
 * ensure that no state leaks from the first MediaFile to the second.
 */

class MediaFileBuilder final {
public:
    MediaFileBuilder(const std::string &filename);
    MediaFileBuilder(const MediaFile &mf);
    MediaFileBuilder(const MediaFileBuilder &) = delete;
    MediaFileBuilder& operator=(MediaFileBuilder &) = delete;

    MediaFile build() const;

    void setType(MediaType t);
    void setETag(const std::string &e);
    void setContentType(const std::string &c);
    void setTitle(const std::string &t);
    void setDate(const std::string &d);
    void setAuthor(const std::string &a);
    void setAlbum(const std::string &a);
    void setAlbumArtist(const std::string &a);
    void setTrackNumber(int n);
    void setDuration(int d);

private:
    bool type_set = false;
    MediaType type = UnknownMedia;

    std::string filename;

    bool content_type_set = false;
    std::string content_type;

    bool etag_set = false;
    std::string etag;

    bool title_set = false;
    std::string title;

    bool date_set = false;
    std::string date;

    bool author_set = false;
    std::string author;

    bool album_set = false;
    std::string album;

    bool album_artist_set = false;
    std::string album_artist;

    int track_number = 0;
    bool track_number_set = false;

    int duration = 0;
    bool duration_set = false;
};

}

#endif
