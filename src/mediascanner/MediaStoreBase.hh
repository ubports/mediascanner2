/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * Authors:
 *    James Henstridge <james.henstridge@canonical.com>
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

#ifndef MEDIASTOREBASE_HH_
#define MEDIASTOREBASE_HH_

#include"scannercore.hh"
#include<vector>
#include<string>

namespace mediascanner {

class MediaFile;
class Album;
class Filter;

class MediaStoreBase {
public:
    MediaStoreBase();
    virtual ~MediaStoreBase();

    MediaStoreBase(const MediaStoreBase &other) = delete;
    MediaStoreBase& operator=(const MediaStoreBase &other) = delete;

    virtual MediaFile lookup(const std::string &filename) const = 0;
    virtual std::vector<MediaFile> query(const std::string &q, MediaType type, const Filter& filter) const = 0;
    virtual std::vector<Album> queryAlbums(const std::string &core_term, const Filter &filter) const = 0;
    virtual std::vector<std::string> queryArtists(const std::string &q, const Filter &filter) const = 0;
    virtual std::vector<MediaFile> getAlbumSongs(const Album& album) const = 0;
    virtual std::string getETag(const std::string &filename) const = 0;
    virtual std::vector<MediaFile> listSongs(const Filter &filter) const = 0;
    virtual std::vector<Album> listAlbums(const Filter &filter) const = 0;
    virtual std::vector<std::string> listArtists(const Filter &filter) const = 0;
    virtual std::vector<std::string>listAlbumArtists(const Filter &filter) const = 0;
    virtual std::vector<std::string>listGenres(const Filter &filter) const = 0;
    virtual bool hasMedia(MediaType type) const = 0;
};

}

#endif
