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
    : QAbstractListModel(parent) {
    roles[Roles::RoleTitle] = "title";
    roles[Roles::RoleArtist] = "artist";
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
        return QString::fromStdString(album.getArtist());
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> AlbumModelBase::roleNames() const {
    return roles;
}

void AlbumModelBase::updateResults(const std::vector<mediascanner::Album> &results) {
    beginResetModel();
    this->results = results;
    endResetModel();
}
