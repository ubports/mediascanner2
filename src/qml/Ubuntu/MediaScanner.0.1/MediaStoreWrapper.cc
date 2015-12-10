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

#include "MediaStoreWrapper.hh"
#include <cstdlib>
#include <cstring>
#include <exception>
#include <QDBusConnection>
#include <QDebug>
#include <QQmlEngine>

#include <core/dbus/asio/executor.h>
#include <mediascanner/Filter.hh>
#include <mediascanner/MediaStore.hh>
#include <ms-dbus/service-stub.hh>

using namespace mediascanner::qml;

static core::dbus::Bus::Ptr the_session_bus() {
    static core::dbus::Bus::Ptr bus = std::make_shared<core::dbus::Bus>(
        core::dbus::WellKnownBus::session);
    static core::dbus::Executor::Ptr executor = core::dbus::asio::make_executor(bus);
    static std::once_flag once;

    std::call_once(once, []() {bus->install_executor(executor);});

    return bus;
}

MediaStoreWrapper::MediaStoreWrapper(QObject *parent)
    : QObject(parent) {
    const char *use_dbus = getenv("MEDIASCANNER_USE_DBUS");
    try {
        if (use_dbus != nullptr && !strcmp(use_dbus, "1")) {
            store.reset(new mediascanner::dbus::ServiceStub(the_session_bus()));
        } else {
            store.reset(new mediascanner::MediaStore(MS_READ_ONLY));
        }
    } catch (const std::exception &e) {
        qWarning() << "Could not initialise media store:" << e.what();
    }

    QDBusConnection::sessionBus().connect(
        "com.canonical.MediaScanner2.Daemon",
        "/com/canonical/unity/scopes",
        "com.canonical.unity.scopes", "InvalidateResults",
        QStringList{"mediascanner-music"}, "s",
        this, SLOT(resultsInvalidated()));
}

QList<QObject*> MediaStoreWrapper::query(const QString &q, MediaType type) {
    if (!store) {
        qWarning() << "query() called on invalid MediaStore";
        return QList<QObject*>();
    }

    QList<QObject*> result;
    try {
        for (const auto &media : store->query(q.toStdString(), static_cast<mediascanner::MediaType>(type), mediascanner::Filter())) {
            auto wrapper = new MediaFileWrapper(media);
            QQmlEngine::setObjectOwnership(wrapper, QQmlEngine::JavaScriptOwnership);
            result.append(wrapper);
        }
    } catch (const std::exception &e) {
        qWarning() << "Failed to retrieve query results:" << e.what();
    }
    return result;
}

MediaFileWrapper *MediaStoreWrapper::lookup(const QString &filename) {
    if (!store) {
        qWarning() << "lookup() called on invalid MediaStore";
        return nullptr;
    }

    MediaFileWrapper *wrapper;
    try {
        wrapper = new MediaFileWrapper(store->lookup(filename.toStdString()));
    } catch (std::exception &e) {
        return nullptr;
    }
    QQmlEngine::setObjectOwnership(wrapper, QQmlEngine::JavaScriptOwnership);
    return wrapper;
}

void MediaStoreWrapper::resultsInvalidated() {
    Q_EMIT updated();
}
