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

#include "service-stub.hh"

#include <stdexcept>

#include <mediascanner/Album.hh>
#include <mediascanner/Filter.hh>
#include <mediascanner/MediaFile.hh>
#include "dbus-interface.hh"
#include "dbus-codec.hh"

using std::string;

namespace mediascanner {
namespace dbus {

struct ServiceStub::Private
{
    core::dbus::Object::Ptr object;
};

ServiceStub::ServiceStub(core::dbus::Bus::Ptr bus)
    : core::dbus::Stub<MediaStoreService>(bus),
      p(new Private{access_service()->object_for_path(
                  core::dbus::types::ObjectPath(core::dbus::traits::Service<MediaStoreService>::object_path()))}) {
}

ServiceStub::~ServiceStub() {
}

MediaFile ServiceStub::lookup(const string &filename) const {
    auto result = p->object->invoke_method_synchronously<MediaStoreInterface::Lookup, MediaFile>(filename);
    if (result.is_error())
        throw std::runtime_error(result.error().print());
    return result.value();
}

std::vector<MediaFile> ServiceStub::query(const string &q, MediaType type, const Filter &filter) const {
    auto result = p->object->invoke_method_synchronously<MediaStoreInterface::Query, std::vector<MediaFile>>(q, (int32_t)type, filter);
    if (result.is_error())
        throw std::runtime_error(result.error().print());
    return result.value();
}

std::vector<Album> ServiceStub::queryAlbums(const string &core_term, const Filter &filter) const {
    auto result = p->object->invoke_method_synchronously<MediaStoreInterface::QueryAlbums, std::vector<Album>>(core_term, filter);
    if (result.is_error())
        throw std::runtime_error(result.error().print());
    return result.value();
}

std::vector<string> ServiceStub::queryArtists(const string &q, const Filter &filter) const {
    auto result = p->object->invoke_method_synchronously<MediaStoreInterface::QueryArtists, std::vector<string>>(q, filter);
    if (result.is_error())
        throw std::runtime_error(result.error().print());
    return result.value();
}

std::vector<MediaFile> ServiceStub::getAlbumSongs(const Album& album) const {
    auto result = p->object->invoke_method_synchronously<MediaStoreInterface::GetAlbumSongs, std::vector<MediaFile>>(album);
    if (result.is_error())
        throw std::runtime_error(result.error().print());
    return result.value();
}

string ServiceStub::getETag(const string &filename) const {
    auto result = p->object->invoke_method_synchronously<MediaStoreInterface::GetETag, string>(filename);
    if (result.is_error())
        throw std::runtime_error(result.error().print());
    return result.value();
}

std::vector<MediaFile> ServiceStub::listSongs(const Filter &filter) const {
    auto result = p->object->invoke_method_synchronously<MediaStoreInterface::ListSongs, std::vector<MediaFile>>(filter);
    if (result.is_error())
        throw std::runtime_error(result.error().print());
    return result.value();
}

std::vector<Album> ServiceStub::listAlbums(const Filter &filter) const {
    auto result = p->object->invoke_method_synchronously<MediaStoreInterface::ListAlbums, std::vector<Album>>(filter);
    if (result.is_error())
        throw std::runtime_error(result.error().print());
    return result.value();
}
std::vector<string> ServiceStub::listArtists(const Filter &filter) const {
    auto result = p->object->invoke_method_synchronously<MediaStoreInterface::ListArtists, std::vector<string>>(filter);
    if (result.is_error())
        throw std::runtime_error(result.error().print());
    return result.value();
}

std::vector<string> ServiceStub::listAlbumArtists(const Filter &filter) const {
    auto result = p->object->invoke_method_synchronously<MediaStoreInterface::ListAlbumArtists, std::vector<string>>(filter);
    if (result.is_error())
        throw std::runtime_error(result.error().print());
    return result.value();
}

std::vector<string> ServiceStub::listGenres(const Filter &filter) const {
    auto result = p->object->invoke_method_synchronously<MediaStoreInterface::ListGenres, std::vector<string>>(filter);
    if (result.is_error())
        throw std::runtime_error(result.error().print());
    return result.value();
}

bool ServiceStub::hasMedia(MediaType type) const {
    auto result = p->object->invoke_method_synchronously<MediaStoreInterface::HasMedia, bool>(static_cast<int32_t>(type));
    if (result.is_error())
        throw std::runtime_error(result.error().print());
    return result.value();
}

}
}
