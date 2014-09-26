/*
 * Copyright (C) 2014 Canonical, Ltd.
 *
 * Authors:
 *    James Henstridge <james.henstridge@canonical.com>
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

#include <functional>
#include <string>

namespace mediascanner {

struct MountWatcherPrivate;

class MountWatcher final {
public:
    struct Info {
        std::string device;
        std::string uuid;
        std::string mount_point;
        bool is_mounted;
    };

    MountWatcher(std::function<void(const Info&)> callback);
    ~MountWatcher();

    MountWatcher(const MountWatcher&) = delete;
    MountWatcher& operator=(MountWatcher& o) = delete;

private:
    MountWatcherPrivate *p;
};

}
