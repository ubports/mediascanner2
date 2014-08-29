/*
 * Copyright (C) 2013 Canonical, Ltd.
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

#include "MediaFileWrapper.hh"

using namespace mediascanner::qml;

MediaFileWrapper::MediaFileWrapper(const mediascanner::MediaFile &media, QObject *parent)
    : QObject(parent), media(media) {
}

QString MediaFileWrapper::filename() const {
    return QString::fromStdString(media.getFileName());
}

QString MediaFileWrapper::uri() const {
    return QString::fromStdString(media.getUri());
}

QString MediaFileWrapper::contentType() const {
    return QString::fromStdString(media.getContentType());
}

QString MediaFileWrapper::eTag() const {
    return QString::fromStdString(media.getETag());
}

QString MediaFileWrapper::title() const {
    return QString::fromStdString(media.getTitle());
}

QString MediaFileWrapper::author() const {
    return QString::fromStdString(media.getAuthor());
}

QString MediaFileWrapper::album() const {
    return QString::fromStdString(media.getAlbum());
}

QString MediaFileWrapper::albumArtist() const {
    return QString::fromStdString(media.getAlbumArtist());
}

QString MediaFileWrapper::date() const {
    return QString::fromStdString(media.getDate());
}

QString MediaFileWrapper::genre() const {
    return QString::fromStdString(media.getGenre());
}

int MediaFileWrapper::discNumber() const {
    return media.getDiscNumber();
}

int MediaFileWrapper::trackNumber() const {
    return media.getTrackNumber();
}

int MediaFileWrapper::duration() const {
    return media.getDuration();
}

int MediaFileWrapper::width() const {
    return media.getWidth();
}

int MediaFileWrapper::height() const {
    return media.getHeight();
}

double MediaFileWrapper::latitude() const {
    return media.getLatitude();
}

double MediaFileWrapper::longitude() const {
    return media.getLongitude();
}

bool MediaFileWrapper::hasThumbnail() const {
    return media.getHasThumbnail();
}

QString MediaFileWrapper::art() const {
    return QString::fromStdString(media.getArtUri());
}
