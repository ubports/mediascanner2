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

#include "ArtistsModel.hh"
#include <QDebug>

using namespace mediascanner::qml;

ArtistsModel::ArtistsModel(QObject *parent)
    : StreamingModel(parent),
      album_artists(false) {
    roles[Roles::RoleArtist] = "artist";
}

int ArtistsModel::rowCount(const QModelIndex &) const {
    return results.size();
}

QVariant ArtistsModel::data(const QModelIndex &index, int role) const {
    if (index.row() < 0 || index.row() >= (ptrdiff_t)results.size()) {
        return QVariant();
    }
    switch (role) {
    case RoleArtist:
        return QString::fromStdString(results[index.row()]);
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> ArtistsModel::roleNames() const {
    return roles;
}

bool ArtistsModel::getAlbumArtists() {
    return album_artists;
}

void ArtistsModel::setAlbumArtists(bool album_artists) {
    if (this->album_artists != album_artists) {
        this->album_artists = album_artists;
        invalidate();
    }
}

QVariant ArtistsModel::getGenre() {
    if (!filter.hasGenre())
        return QVariant();
    return QString::fromStdString(filter.getGenre());
}

void ArtistsModel::setGenre(const QVariant genre) {
    if (genre.isNull()) {
        if (filter.hasGenre()) {
            filter.unsetGenre();
            invalidate();
        }
    } else {
        const std::string std_genre = genre.value<QString>().toStdString();
        if (!filter.hasGenre() || filter.getGenre() != std_genre) {
            filter.setGenre(std_genre);
            invalidate();
        }
    }
}

int ArtistsModel::getLimit() {
    return -1;
}

void ArtistsModel::setLimit(int) {
    qWarning() << "Setting limit on ArtistsModel is deprecated";
}

namespace {
class ArtistRowData : public StreamingModel::RowData {
public:
    ArtistRowData(std::vector<std::string> &&rows) : rows(std::move(rows)) {}
    ~ArtistRowData() {}
    size_t size() const override { return rows.size(); }
    std::vector<std::string> rows;
};
}

std::unique_ptr<StreamingModel::RowData> ArtistsModel::retrieveRows(std::shared_ptr<MediaStoreBase> store, int limit, int offset) const {
    auto limit_filter = filter;
    limit_filter.setLimit(limit);
    limit_filter.setOffset(offset);
    std::vector<std::string> artists;
    if (album_artists) {
        artists = store->listAlbumArtists(limit_filter);
    } else {
        artists = store->listArtists(limit_filter);
    }
    return std::unique_ptr<StreamingModel::RowData>(
        new ArtistRowData(std::move(artists)));
}

void ArtistsModel::appendRows(std::unique_ptr<StreamingModel::RowData> &&row_data) {
    ArtistRowData *data = static_cast<ArtistRowData*>(row_data.get());
    std::move(data->rows.begin(), data->rows.end(), std::back_inserter(results));
}

void ArtistsModel::clearBacking() {
    results.clear();
}
