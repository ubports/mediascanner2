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

#ifndef MEDIASCANNER_QML_GENRESMODEL_H
#define MEDIASCANNER_QML_GENRESMODEL_H

#include <string>
#include <QAbstractListModel>
#include <QString>

#include <mediascanner/Filter.hh>
#include "MediaStoreWrapper.hh"

namespace mediascanner {
namespace qml {

class GenresModel : public QAbstractListModel {
    Q_OBJECT
    Q_ENUMS(Roles)
    Q_PROPERTY(mediascanner::qml::MediaStoreWrapper* store READ getStore WRITE setStore)
    Q_PROPERTY(int limit READ getLimit WRITE setLimit)
    Q_PROPERTY(int rowCount READ rowCount) // NOTIFY modelReset
public:
    enum Roles {
        RoleGenre,
    };

    explicit GenresModel(QObject *parent = 0);
    int rowCount(const QModelIndex &parent=QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    Q_INVOKABLE QVariant get(int row, Roles role) const;
protected:
    QHash<int, QByteArray> roleNames() const override;

    MediaStoreWrapper *getStore();
    void setStore(MediaStoreWrapper *store);
    int getLimit();
    void setLimit(int limit);

private:
    void update();

    QHash<int, QByteArray> roles;
    std::vector<std::string> results;
    MediaStoreWrapper *store;
    int limit;
};

}
}

#endif
