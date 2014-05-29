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

#ifndef MEDIASCANNER_DBUS_SERVICE_SKELETON_HH
#define MEDIASCANNER_DBUS_SERVICE_SKELETON_HH

#include <memory>
#include <core/dbus/bus.h>
#include <core/dbus/skeleton.h>

#include "service.hh"

namespace mediascanner {

class MediaStore;

namespace dbus {

class ServiceSkeleton : public core::dbus::Skeleton<MediaStoreService> {
public:
    ServiceSkeleton(core::dbus::Bus::Ptr bus, std::shared_ptr<MediaStore> store);
    ~ServiceSkeleton();

    void run();
    void stop();

private:
    struct Private;
    std::unique_ptr<Private> p;
};

}
}

#endif
