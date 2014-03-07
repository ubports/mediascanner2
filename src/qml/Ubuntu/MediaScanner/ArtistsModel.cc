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

using namespace mediascanner::qml;

ArtistsModel::ArtistsModel(QObject *parent)
    : QAbstractListModel(parent),
      store(nullptr),
      album_artists(false),
      limit(-1) {
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

MediaStoreWrapper *ArtistsModel::getStore() {
    return store;
}

void ArtistsModel::setStore(MediaStoreWrapper *store) {
    if (this->store != store) {
        this->store = store;
        update();
    }
}

bool ArtistsModel::getAlbumArtists() {
    return album_artists;
}

void ArtistsModel::setAlbumArtists(bool album_artists) {
    if (this->album_artists != album_artists) {
        this->album_artists = album_artists;
        update();
    }
}

int ArtistsModel::getLimit() {
    return limit;
}

void ArtistsModel::setLimit(int limit) {
    if (this->limit != limit) {
        this->limit = limit;
        update();
    }
}

void ArtistsModel::update() {
    beginResetModel();
    if (store == nullptr) {
        this->results.clear();
    } else {
        this->results = store->store.listArtists(album_artists, limit);
    }
    this->results = results;
    endResetModel();
}
