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

#ifndef MEDIASCANNER_QML_ALBUMSMODEL_H
#define MEDIASCANNER_QML_ALBUMSMODEL_H

#include <QString>

#include <mediascanner/Filter.hh>
#include "MediaStoreWrapper.hh"
#include "AlbumModelBase.hh"

namespace mediascanner {
namespace qml {

class AlbumsModel : public AlbumModelBase {
    Q_OBJECT
    Q_PROPERTY(QVariant artist READ getArtist WRITE setArtist)
    Q_PROPERTY(QVariant albumArtist READ getAlbumArtist WRITE setAlbumArtist)
    Q_PROPERTY(QVariant genre READ getGenre WRITE setGenre)
    Q_PROPERTY(int limit READ getLimit WRITE setLimit)
public:
    explicit AlbumsModel(QObject *parent=0);

    std::unique_ptr<RowData> retrieveRows(std::shared_ptr<mediascanner::MediaStoreBase> store, int limit, int offset) const override;

protected:
    QVariant getArtist();
    void setArtist(const QVariant artist);
    QVariant getAlbumArtist();
    void setAlbumArtist(const QVariant album_artist);
    QVariant getGenre();
    void setGenre(const QVariant genre);
    int getLimit();
    void setLimit(int limit);

private:
    Filter filter;
};

}
}

#endif
