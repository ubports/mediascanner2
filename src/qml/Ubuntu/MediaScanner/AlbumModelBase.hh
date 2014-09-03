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
#include "StreamingModel.hh"

namespace mediascanner {
namespace qml {

class AlbumModelBase : public StreamingModel {
    Q_OBJECT
    Q_ENUMS(Roles)
public:
    enum Roles {
        RoleTitle,
        RoleArtist,
        RoleDate,
        RoleGenre,
        RoleArt,
    };

    explicit AlbumModelBase(QObject *parent = 0);
    int rowCount(const QModelIndex &parent=QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    void appendRows(std::unique_ptr<RowData> &&row_data) override;
    void clearBacking() override;

    class AlbumRowData : public RowData {
    public:
        AlbumRowData(std::vector<mediascanner::Album> &&rows) : rows(std::move(rows)) {}
        ~AlbumRowData() {}
        size_t size() const override { return rows.size(); }
        std::vector<mediascanner::Album> rows;
    };

protected:
    QHash<int, QByteArray> roleNames() const override;
private:
    QHash<int, QByteArray> roles;
    std::vector<mediascanner::Album> results;
};

}
}

#endif
