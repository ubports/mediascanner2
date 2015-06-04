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
#include <QDebug>

using namespace mediascanner::qml;

SongsModel::SongsModel(QObject *parent)
    : MediaFileModelBase(parent) {
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
            invalidate();
        }
    } else {
        const std::string std_artist = artist.value<QString>().toStdString();
        if (!filter.hasArtist() || filter.getArtist() != std_artist) {
            filter.setArtist(std_artist);
            invalidate();
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
            invalidate();
        }
    } else {
        const std::string std_album = album.value<QString>().toStdString();
        if (!filter.hasAlbum() || filter.getAlbum() != std_album) {
            filter.setAlbum(std_album);
            invalidate();
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
            invalidate();
        }
    } else {
        const std::string std_album_artist = album_artist.value<QString>().toStdString();
        if (!filter.hasAlbumArtist() || filter.getAlbumArtist() != std_album_artist) {
            filter.setAlbumArtist(std_album_artist);
            invalidate();
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

int SongsModel::getLimit() {
    return -1;
}

void SongsModel::setLimit(int) {
    qWarning() << "Setting limit on SongsModel is deprecated";
}

std::unique_ptr<StreamingModel::RowData> SongsModel::retrieveRows(std::shared_ptr<MediaStoreBase> store, int limit, int offset) const {
    auto limit_filter = filter;
    limit_filter.setLimit(limit);
    limit_filter.setOffset(offset);
    return std::unique_ptr<StreamingModel::RowData>(
        new MediaFileRowData(store->listSongs(limit_filter)));
}
