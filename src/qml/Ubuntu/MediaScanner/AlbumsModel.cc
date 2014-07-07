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
#include <exception>
#include <QDebug>

using namespace mediascanner::qml;

AlbumsModel::AlbumsModel(QObject *parent)
    : AlbumModelBase(parent),
      store(nullptr) {
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

QVariant AlbumsModel::getArtist() {
    if (!filter.hasArtist())
        return QVariant();
    return QString::fromStdString(filter.getArtist());
}

void AlbumsModel::setArtist(const QVariant artist) {
    if (artist.isNull()) {
        if (filter.hasArtist()) {
            filter.unsetArtist();
            update();
        }
    } else {
        const std::string std_artist = artist.value<QString>().toStdString();
        if (!filter.hasArtist() || filter.getArtist() != std_artist) {
            filter.setArtist(std_artist);
            update();
        }
    }
}

QVariant AlbumsModel::getAlbumArtist() {
    if (!filter.hasAlbumArtist())
        return QVariant();
    return QString::fromStdString(filter.getAlbumArtist());
}

void AlbumsModel::setAlbumArtist(const QVariant album_artist) {
    if (album_artist.isNull()) {
        if (filter.hasAlbumArtist()) {
            filter.unsetAlbumArtist();
            update();
        }
    } else {
        const std::string std_album_artist = album_artist.value<QString>().toStdString();
        if (!filter.hasAlbumArtist() || filter.getAlbumArtist() != std_album_artist) {
            filter.setAlbumArtist(std_album_artist);
            update();
        }
    }
}

QVariant AlbumsModel::getGenre() {
    if (!filter.hasGenre())
        return QVariant();
    return QString::fromStdString(filter.getGenre());
}

void AlbumsModel::setGenre(const QVariant genre) {
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

int AlbumsModel::getLimit() {
    return filter.getLimit();
}

void AlbumsModel::setLimit(int limit) {
    if (filter.getLimit() != limit) {
        filter.setLimit(limit);
        update();
    }
}

void AlbumsModel::update() {
    if (store == nullptr) {
        updateResults(std::vector<mediascanner::Album>());
    } else {
        try {
            updateResults(store->store->listAlbums(filter));
        } catch (const std::exception &e) {
            qWarning() << "Failed to retrieve album list:" << e.what();
            updateResults(std::vector<mediascanner::Album>());
        }
    }
}
