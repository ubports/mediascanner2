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
#include <QQmlEngine>

using namespace mediascanner::qml;

MediaStoreWrapper::MediaStoreWrapper(QObject *parent)
    : QObject(parent), store(MS_READ_ONLY) {
}

QList<QObject*> MediaStoreWrapper::query(const QString &q, MediaType type) {
    QList<QObject*> result;
    for (const auto &media : store.query(q.toStdString(), static_cast<mediascanner::MediaType>(type))) {
        auto wrapper = new MediaFileWrapper(media);
        QQmlEngine::setObjectOwnership(wrapper, QQmlEngine::JavaScriptOwnership);
        result.append(wrapper);
    }
    return result;
}

MediaFileWrapper *MediaStoreWrapper::lookup(const QString &filename) {
    MediaFileWrapper *wrapper;
    try {
        wrapper = new MediaFileWrapper(store.lookup(filename.toStdString()));
    } catch (std::runtime_error &e) {
        return nullptr;
    }
    QQmlEngine::setObjectOwnership(wrapper, QQmlEngine::JavaScriptOwnership);
    return wrapper;
}
