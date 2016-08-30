/*
 * Copyright (C) 2014 Canonical, Ltd.
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

#ifndef INVALIDATIONSENDER_HH
#define INVALIDATIONSENDER_HH

#include <memory>

typedef struct _GDBusConnection GDBusConnection;

namespace mediascanner {

/**
 * A class that sends a broadcast signal that the state of media
 * files has changed.
 */

class InvalidationSender final {
public:
    InvalidationSender();
    ~InvalidationSender();
    InvalidationSender(const InvalidationSender &o) = delete;
    InvalidationSender& operator=(const InvalidationSender &o) = delete;

    void invalidate();
    void setBus(GDBusConnection *bus);
    void setDelay(int delay);

private:
    static int callback(void *data);

    std::unique_ptr<GDBusConnection, void(*)(void*)> bus;
    unsigned int timeout_id = 0;
    int delay = 0;
};

}

#endif
