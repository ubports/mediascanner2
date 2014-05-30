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

#ifndef MEDIASCANNER_DBUS_SERVICE_HH
#define MEDIASCANNER_DBUS_SERVICE_HH

#include <core/dbus/traits/service.h>

namespace mediascanner {
namespace dbus {

class MediaStoreService {
public:
    MediaStoreService() {}

    MediaStoreService(const MediaStoreService&) = delete;
    virtual ~MediaStoreService() = default;

    MediaStoreService& operator=(const MediaStoreService&) = delete;
    bool operator==(const MediaStoreService&) const = delete;
};

}
}

namespace core {
namespace dbus {
namespace traits {

template<>
struct Service<mediascanner::dbus::MediaStoreService> {
    inline static const std::string& interface_name() {
        static const std::string iface("com.canonical.MediaScanner2");
        return iface;
    }
    inline static const std::string& object_path() {
        static const std::string path("/com/canonical/MediaScanner2");
        return path;
    }
};

}
}
}

#endif
