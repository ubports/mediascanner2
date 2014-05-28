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

#include "AlbumsModel.hh"

using namespace mediascanner::qml;

AlbumsModel::AlbumsModel(QObject *parent)
    : AlbumModelBase(parent),
      store(nullptr),
      limit(-1) {
}

MediaStoreWrapper *AlbumsModel::getStore() {
    return store;
}

void AlbumsModel::setStore(MediaStoreWrapper *store) {
    if (this->store != store) {
        this->store = store;
        update();
    }
}

QString AlbumsModel::getArtist() {
    return QString::fromStdString(filter.getArtist());
}

void AlbumsModel::setArtist(const QString artist) {
    const std::string std_artist = artist.toStdString();
    if (filter.getArtist() != std_artist) {
        filter.setArtist(std_artist);
        update();
    }
}

QString AlbumsModel::getAlbumArtist() {
    return QString::fromStdString(filter.getAlbumArtist());
}

void AlbumsModel::setAlbumArtist(const QString album_artist) {
    const std::string std_album_artist = album_artist.toStdString();
    if (filter.getAlbumArtist() != std_album_artist) {
        filter.setAlbumArtist(std_album_artist);
        update();
    }
}

int AlbumsModel::getLimit() {
    return limit;
}

void AlbumsModel::setLimit(int limit) {
    if (this->limit != limit) {
        this->limit = limit;
        update();
    }
}

void AlbumsModel::update() {
    if (store == nullptr) {
        updateResults(std::vector<mediascanner::Album>());
    } else {
        updateResults(store->store.listAlbums(filter, limit));
    }
}
