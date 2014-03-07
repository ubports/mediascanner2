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

#ifndef MEDIASCANNER_QML_ALBUMMODELBASE_H
#define MEDIASCANNER_QML_ALBUMMODELBASE_H

#include <QAbstractListModel>
#include <QString>

#include <mediascanner/Album.hh>

namespace mediascanner {
namespace qml {

class AlbumModelBase : public QAbstractListModel {
    Q_OBJECT
    Q_ENUMS(Roles)
public:
    enum Roles {
        RoleTitle,
        RoleArtist,
    };

    explicit AlbumModelBase(QObject *parent = 0);
    int rowCount(const QModelIndex &parent=QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
protected:
    QHash<int, QByteArray> roleNames() const override;
    void updateResults(const std::vector<mediascanner::Album> &results);
private:
    QHash<int, QByteArray> roles;
    std::vector<mediascanner::Album> results;
};

}
}

#endif
