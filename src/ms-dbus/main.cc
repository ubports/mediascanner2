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
#include <core/dbus/bus.h>
#include <core/dbus/asio/executor.h>

#include <mediascanner/MediaStore.hh>
#include "service-skeleton.hh"

using namespace mediascanner;

int main(int , char **) {
    auto bus = std::make_shared<core::dbus::Bus>(core::dbus::WellKnownBus::session);
    bus->install_executor(core::dbus::asio::make_executor(bus));

    auto store = std::make_shared<MediaStore>(MS_READ_ONLY);

    dbus::ServiceSkeleton service(bus, store);
    service.run();
    return 0;
}
