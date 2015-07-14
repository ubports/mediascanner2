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

#ifndef MEDIASCANNER_DBUS_SERVICE_STUB_HH
#define MEDIASCANNER_DBUS_SERVICE_STUB_HH

#include <memory>
#include <string>
#include <vector>
#include <core/dbus/bus.h>
#include <core/dbus/stub.h>

#include <mediascanner/MediaStoreBase.hh>
#include "service.hh"

namespace mediascanner {

class Album;
class Filter;
class MediaFile;

namespace dbus {

class ServiceStub : public core::dbus::Stub<MediaStoreService>, public virtual MediaStoreBase {
public:
    explicit ServiceStub(core::dbus::Bus::Ptr bus);
    virtual ~ServiceStub();

    virtual MediaFile lookup(const std::string &filename) const override;
    virtual std::vector<MediaFile> query(const std::string &q, MediaType type, const Filter &filter) const override;
    virtual std::vector<Album> queryAlbums(const std::string &core_term, const Filter &filter) const override;
    virtual std::vector<std::string> queryArtists(const std::string &q, const Filter &filter) const override;
    virtual std::vector<MediaFile> getAlbumSongs(const Album& album) const override;
    virtual std::string getETag(const std::string &filename) const override;
    virtual std::vector<MediaFile> listSongs(const Filter &filter) const override;
    virtual std::vector<Album> listAlbums(const Filter &filter) const override;
    virtual std::vector<std::string> listArtists(const Filter &filter) const override;
    virtual std::vector<std::string> listAlbumArtists(const Filter &filter) const override;
    virtual std::vector<std::string> listGenres(const Filter &filter) const override;
    virtual bool hasMedia(MediaType type) const override;

private:
    struct Private;
    std::unique_ptr<Private> p;
};

}
}

#endif
