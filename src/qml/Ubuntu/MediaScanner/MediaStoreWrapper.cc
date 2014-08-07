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
#include <mediascanner/MediaStore.hh>
#include <ms-dbus/service-stub.hh>

using namespace mediascanner::qml;

static const char SCANNER_BUS_NAME[] = "com.canonical.MediaScanner2.Daemon";
static const char SCANNER_IFACE[] = "com.canonical.unity.scopes";
static const char SCANNER_PATH[] = "/com/canonical/unity/scopes";
static const char INVALIDATE_RESULTS[] = "InvalidateResults";

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
    if (use_dbus != nullptr && !strcmp(use_dbus, "1")) {
        store.reset(new mediascanner::dbus::ServiceStub(the_session_bus()));
    } else {
        store.reset(new mediascanner::MediaStore(MS_READ_ONLY));
    }

    QDBusConnection::sessionBus().connect(
        SCANNER_BUS_NAME, SCANNER_PATH,
        SCANNER_IFACE, INVALIDATE_RESULTS, "s",
        this, SLOT(resultsInvalidated(QString)));
}

MediaStoreWrapper::~MediaStoreWrapper() {
    QDBusConnection::sessionBus().disconnect(
        SCANNER_BUS_NAME, SCANNER_PATH,
        SCANNER_IFACE, INVALIDATE_RESULTS, "s",
        this, SLOT(resultsInvalidated(QString)));
}

QList<QObject*> MediaStoreWrapper::query(const QString &q, MediaType type) {
    QList<QObject*> result;
    try {
        for (const auto &media : store->query(q.toStdString(), static_cast<mediascanner::MediaType>(type))) {
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
    MediaFileWrapper *wrapper;
    try {
        wrapper = new MediaFileWrapper(store->lookup(filename.toStdString()));
    } catch (std::exception &e) {
        return nullptr;
    }
    QQmlEngine::setObjectOwnership(wrapper, QQmlEngine::JavaScriptOwnership);
    return wrapper;
}

void MediaStoreWrapper::resultsInvalidated(const QString &scopeName) {
    if (scopeName == "mediascanner-music") {
        Q_EMIT updated();
    }
}
