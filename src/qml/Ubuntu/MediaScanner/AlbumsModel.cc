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
      artist(""),
      album_artist(""),
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
    return artist;
}

void AlbumsModel::setArtist(const QString artist) {
    if (this->artist != artist) {
        this->artist = artist;
        update();
    }
}

QString AlbumsModel::getAlbumArtist() {
    return album_artist;
}

void AlbumsModel::setAlbumArtist(const QString album_artist) {
    if (this->album_artist != album_artist) {
        this->album_artist = album_artist;
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
        updateResults(store->store.listAlbums(artist.toStdString(), album_artist.toStdString(), limit));
    }
}
