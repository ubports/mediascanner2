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

#ifndef MEDIASCANNER_QML_MEDIASTOREWRAPPER_H
#define MEDIASCANNER_QML_MEDIASTOREWRAPPER_H

#include <QList>
#include <QObject>
#include <QString>

#include <ms-dbus/service-stub.hh>
#include "MediaFileWrapper.hh"

namespace mediascanner {

namespace qml {

class MediaStoreWrapper : public QObject {
    Q_OBJECT
    Q_ENUMS(MediaType)
public:
    enum MediaType {
        AudioMedia = mediascanner::AudioMedia,
        VideoMedia = mediascanner::VideoMedia,
        ImageMedia = mediascanner::ImageMedia,
        AllMedia = mediascanner::AllMedia,
    };
    MediaStoreWrapper(QObject *parent=0);

    Q_INVOKABLE QList<QObject*> query(const QString &q, MediaType type);
    Q_INVOKABLE mediascanner::qml::MediaFileWrapper *lookup(const QString &filename);

    mediascanner::dbus::ServiceStub store;
};

}
}

#endif
