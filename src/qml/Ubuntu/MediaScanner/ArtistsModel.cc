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
#include <exception>
#include <QDebug>

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

QVariant ArtistsModel::get(int row, ArtistsModel::Roles role) const {
    return data(index(row, 0), role);
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

QVariant ArtistsModel::getGenre() {
    if (!filter.hasGenre())
        return QVariant();
    return QString::fromStdString(filter.getGenre());
}

void ArtistsModel::setGenre(const QVariant genre) {
    if (genre.isNull()) {
        if (filter.hasGenre()) {
            filter.unsetGenre();
            update();
        }
    } else {
        const std::string std_genre = genre.value<QString>().toStdString();
        if (!filter.hasGenre() || filter.getGenre() != std_genre) {
            filter.setGenre(std_genre);
            update();
        }
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
        try {
            if (album_artists) {
                this->results = store->store->listAlbumArtists(filter, limit);
            } else {
                this->results = store->store->listArtists(filter, limit);
            }
        } catch (const std::exception &e) {
            qWarning() << "Failed to retrieve artist list:" << e.what();
            this->results.clear();
        }
    }
    endResetModel();
    Q_EMIT rowCountChanged();
}
