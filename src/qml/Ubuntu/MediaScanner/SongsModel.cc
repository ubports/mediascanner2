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

#include "SongsModel.hh"

using namespace mediascanner::qml;

SongsModel::SongsModel(QObject *parent)
    : MediaFileModelBase(parent),
      store(nullptr),
      limit(-1) {
}

MediaStoreWrapper *SongsModel::getStore() {
    return store;
}

void SongsModel::setStore(MediaStoreWrapper *store) {
    if (this->store != store) {
        this->store = store;
        update();
    }
}

QString SongsModel::getArtist() {
    return QString::fromStdString(filter.getArtist());
}

void SongsModel::setArtist(const QString artist) {
    const std::string std_artist = artist.toStdString();
    if (filter.getArtist() != std_artist) {
        filter.setArtist(std_artist);
        update();
    }
}

QString SongsModel::getAlbum() {
    return QString::fromStdString(filter.getAlbum());
}

void SongsModel::setAlbum(const QString album) {
    const std::string std_album = album.toStdString();
    if (filter.getAlbum() != std_album) {
        filter.setAlbum(std_album);
        update();
    }
}

QString SongsModel::getAlbumArtist() {
    return QString::fromStdString(filter.getAlbumArtist());
}

void SongsModel::setAlbumArtist(const QString album_artist) {
    const std::string std_album_artist = album_artist.toStdString();
    if (filter.getAlbumArtist() != std_album_artist) {
        filter.setAlbumArtist(std_album_artist);
        update();
    }
}

int SongsModel::getLimit() {
    return limit;
}

void SongsModel::setLimit(int limit) {
    if (this->limit != limit) {
        this->limit = limit;
        update();
    }
}

void SongsModel::update() {
    if (store == nullptr) {
        updateResults(std::vector<mediascanner::MediaFile>());
    } else {
        updateResults(store->store.listSongs(filter, limit));
    }
}
