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

#include "AlbumModelBase.hh"

using namespace mediascanner::qml;

AlbumModelBase::AlbumModelBase(QObject *parent)
    : StreamingModel(parent) {
    roles[Roles::RoleTitle] = "title";
    roles[Roles::RoleArtist] = "artist";
    roles[Roles::RoleDate] = "date";
    roles[Roles::RoleGenre] = "genre";
    roles[Roles::RoleArt] = "art";
}

int AlbumModelBase::rowCount(const QModelIndex &) const {
    return results.size();
}

QVariant AlbumModelBase::data(const QModelIndex &index, int role) const {
    if (index.row() < 0 || index.row() >= (ptrdiff_t)results.size()) {
        return QVariant();
    }
    const mediascanner::Album &album = results[index.row()];
    switch (role) {
    case RoleTitle:
        return QString::fromStdString(album.getTitle());
    case RoleArtist:
        if (album.getArtistCount() > 1) {
            return QStringLiteral("Various"); // This gets translated in the client apps
        }

        return QString::fromStdString(album.getArtist());
    case RoleDate:
        return QString::fromStdString(album.getDate());
    case RoleGenre:
        return QString::fromStdString(album.getGenre());
    case RoleArt:
        return QString::fromStdString(album.getArtUri());
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> AlbumModelBase::roleNames() const {
    return roles;
}

void AlbumModelBase::appendRows(std::unique_ptr<RowData> &&row_data) {
    AlbumRowData *data = static_cast<AlbumRowData*>(row_data.get());
    std::move(data->rows.begin(), data->rows.end(), std::back_inserter(results));
}

void AlbumModelBase::clearBacking() {
    results.clear();
}
