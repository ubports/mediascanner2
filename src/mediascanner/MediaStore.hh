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

#ifndef MEDIASTORE_HH_
#define MEDIASTORE_HH_

#include"scannercore.hh"
#include<vector>
#include<string>

struct MediaStorePrivate;
class MediaFile;
class Album;

enum OpenType {
    MS_READ_ONLY,
    MS_READ_WRITE
};

class MediaStore final {
private:
    MediaStorePrivate *p;

public:
    MediaStore(const std::string &filename, OpenType access, const std::string &retireprefix="");
    MediaStore(const MediaStore &other) = delete;
    MediaStore operator=(const MediaStore &other) = delete;
    ~MediaStore();

    void insert(const MediaFile &m);
    void remove(const std::string &fname);
    std::vector<MediaFile> query(const std::string &q, MediaType type);
    std::vector<Album> queryAlbums(const std::string &core_term);
    std::vector<MediaFile> getAlbumSongs(const Album& album);
    size_t size() const;
    void pruneDeleted();
    void archiveItems(const std::string &prefix);
    void restoreItems(const std::string &prefix);
};

#endif