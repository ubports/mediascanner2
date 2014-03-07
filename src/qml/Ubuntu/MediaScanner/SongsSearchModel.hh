/*
 * Copyright (C) 2014 Canonical, Ltd.
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

#ifndef MEDIASCANNER_QML_SONGSSEARCHMODEL_H
#define MEDIASCANNER_QML_SONGSSEARCHMODEL_H

#include <QString>

#include "MediaStoreWrapper.hh"
#include "MediaFileModelBase.hh"

namespace mediascanner {
namespace qml {

class SongsSearchModel : public MediaFileModelBase {
    Q_OBJECT
    Q_PROPERTY(mediascanner::qml::MediaStoreWrapper* store READ getStore WRITE setStore)
    Q_PROPERTY(QString query READ getQuery WRITE setQuery)
public:
    explicit SongsSearchModel(QObject *parent=0);

    MediaStoreWrapper *getStore();
    void setStore(MediaStoreWrapper *store);

    QString getQuery();
    void setQuery(const QString query);
private:
    void update();

    MediaStoreWrapper *store;
    QString query;
};

}
}

#endif
