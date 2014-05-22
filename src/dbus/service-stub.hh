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

#include <memory>
#include <string>
#include <vector>
#include <core/dbus/bus.h>
#include <core/dbus/stub.h>

#include "service.hh"

namespace mediascanner {

class Album;
class MediaFile;

namespace dbus {

class ServiceStub : public core::dbus::Stub<ScannerService> {
public:
    ServiceStub(core::dbus::Bus::Ptr bus);
    ~ServiceStub();

    MediaFile lookup(const std::string &filename) const;
    std::vector<MediaFile> query(const std::string &q, MediaType type, int limit=-1) const;
    std::vector<Album> queryAlbums(const std::string &core_term, int limit=-1) const;
    std::vector<MediaFile> getAlbumSongs(const Album& album) const;
    std::string getETag(const std::string &filename) const;
    std::vector<MediaFile> listSongs(const std::string& artist="", const std::string& album="", const std::string& album_artist="", int limit=-1) const;
    std::vector<Album> listAlbums(const std::string& artist="", const std::string& album_artist="", int limit=-1) const;
    std::vector<std::string> listArtists(bool album_artists, int limit=-1) const;

private:
    struct Private;
    std::unique_ptr<Private> p;
};

}
}
