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

#include "MediaFileModelBase.hh"
#include "MediaFileWrapper.hh"
#include "utils.hh"

using namespace mediascanner::qml;

MediaFileModelBase::MediaFileModelBase(QObject *parent)
    : QAbstractListModel(parent) {
    roles[Roles::RoleModelData] = "modelData";
    roles[Roles::RoleFilename] = "filename";
    roles[Roles::RoleUri] = "uri";
    roles[Roles::RoleContentType] = "contentType";
    roles[Roles::RoleETag] = "eTag";
    roles[Roles::RoleTitle] = "title";
    roles[Roles::RoleAuthor] = "author";
    roles[Roles::RoleAlbum] = "album";
    roles[Roles::RoleAlbumArtist] = "albumArtist";
    roles[Roles::RoleDate] = "date";
    roles[Roles::RoleGenre] = "genre";
    roles[Roles::RoleDiscNumber] = "discNumber";
    roles[Roles::RoleTrackNumber] = "trackNumber";
    roles[Roles::RoleDuration] = "duration";
    roles[Roles::RoleWidth] = "width";
    roles[Roles::RoleHeight] = "height";
    roles[Roles::RoleLatitude] = "latitude";
    roles[Roles::RoleLongitude] = "longitude";
    roles[Roles::RoleArt] = "art";
}

int MediaFileModelBase::rowCount(const QModelIndex &) const {
    return results.size();
}

QVariant MediaFileModelBase::data(const QModelIndex &index, int role) const {
    if (index.row() < 0 || index.row() >= (ptrdiff_t)results.size()) {
        return QVariant();
    }
    const mediascanner::MediaFile &media = results[index.row()];
    switch (role) {
    case RoleModelData:
        return QVariant::fromValue(new MediaFileWrapper(media));
    case RoleFilename:
        return QString::fromStdString(media.getFileName());
    case RoleUri:
        return QString::fromStdString(media.getUri());
    case RoleContentType:
        return QString::fromStdString(media.getContentType());
    case RoleETag:
        return QString::fromStdString(media.getETag());
    case RoleTitle:
        return QString::fromStdString(media.getTitle());
    case RoleAuthor:
        return QString::fromStdString(media.getAuthor());
    case RoleAlbum:
        return QString::fromStdString(media.getAlbum());
    case RoleAlbumArtist:
        return QString::fromStdString(media.getAlbumArtist());
    case RoleDate:
        return QString::fromStdString(media.getDate());
    case RoleGenre:
        return QString::fromStdString(media.getGenre());
    case RoleDiscNumber:
        return media.getDiscNumber();
    case RoleTrackNumber:
        return media.getTrackNumber();
    case RoleDuration:
        return media.getDuration();
    case RoleWidth:
        return media.getWidth();
    case RoleHeight:
        return media.getHeight();
    case RoleLatitude:
        return media.getLatitude();
    case RoleLongitude:
        return media.getLongitude();
    case RoleArt:
        return make_album_art_uri(media.getAuthor(), media.getAlbum());
    default:
        return QVariant();
    }
}

QVariant MediaFileModelBase::get(int row, MediaFileModelBase::Roles role) const {
    return data(index(row, 0), role);
}

QHash<int, QByteArray> MediaFileModelBase::roleNames() const {
    return roles;
}

void MediaFileModelBase::updateResults(const std::vector<mediascanner::MediaFile> &results) {
    beginResetModel();
    this->results = results;
    endResetModel();
    Q_EMIT rowCountChanged();
}