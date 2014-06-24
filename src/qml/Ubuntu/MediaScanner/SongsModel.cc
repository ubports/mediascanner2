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
#include <exception>
#include <QDebug>

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

QVariant SongsModel::getArtist() {
    if (!filter.hasArtist())
        return QVariant();
    return QString::fromStdString(filter.getArtist());
}

void SongsModel::setArtist(const QVariant artist) {
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

QVariant SongsModel::getAlbum() {
    if (!filter.hasAlbum())
        return QVariant();
    return QString::fromStdString(filter.getAlbum());
}

void SongsModel::setAlbum(const QVariant album) {
    if (album.isNull()) {
        if (filter.hasAlbum()) {
            filter.unsetAlbum();
            update();
        }
    } else {
        const std::string std_album = album.value<QString>().toStdString();
        if (!filter.hasAlbum() || filter.getAlbum() != std_album) {
            filter.setAlbum(std_album);
            update();
        }
    }
}

QVariant SongsModel::getAlbumArtist() {
    if (!filter.hasAlbumArtist())
        return QVariant();
    return QString::fromStdString(filter.getAlbumArtist());
}

void SongsModel::setAlbumArtist(const QVariant album_artist) {
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

QVariant SongsModel::getGenre() {
    if (!filter.hasGenre())
        return QVariant();
    return QString::fromStdString(filter.getGenre());
}

void SongsModel::setGenre(const QVariant genre) {
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
        try {
            updateResults(store->store->listSongs(filter, limit));
        } catch (const std::exception &e) {
            qWarning() << "Failed to retrieve songs list:" << e.what();
            updateResults(std::vector<mediascanner::MediaFile>());
        }
    }
}
