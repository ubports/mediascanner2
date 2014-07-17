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

#ifndef MEDIASCANNER_QML_ARTISTSMODEL_H
#define MEDIASCANNER_QML_ARTISTSMODEL_H

#include <string>
#include <QAbstractListModel>
#include <QString>

#include <mediascanner/Filter.hh>
#include "StreamingModel.hh"

namespace mediascanner {
namespace qml {

class ArtistsModel : public StreamingModel {
    Q_OBJECT
    Q_ENUMS(Roles)
    Q_PROPERTY(bool albumArtists READ getAlbumArtists WRITE setAlbumArtists)
    Q_PROPERTY(QVariant genre READ getGenre WRITE setGenre)
    Q_PROPERTY(int limit READ getLimit WRITE setLimit)
public:
    enum Roles {
        RoleArtist,
    };

    explicit ArtistsModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent=QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    std::unique_ptr<RowData> retrieveRows(std::shared_ptr<mediascanner::MediaStoreBase> store, int limit, int offset) const override;
    void appendRows(std::unique_ptr<RowData> &&row_data) override;
    void clearBacking() override;

protected:
    QHash<int, QByteArray> roleNames() const override;

    bool getAlbumArtists();
    void setAlbumArtists(bool album_artists);
    QVariant getGenre();
    void setGenre(QVariant genre);
    int getLimit();
    void setLimit(int limit);

private:
    QHash<int, QByteArray> roles;
    std::vector<std::string> results;
    Filter filter;
    bool album_artists;
};

}
}

#endif
