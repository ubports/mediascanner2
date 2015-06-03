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

#ifndef MEDIASCANNER_QML_MEDIAFILEWRAPPER_H
#define MEDIASCANNER_QML_MEDIAFILEWRAPPER_H

#include <QObject>
#include <QString>

#include <mediascanner/MediaFile.hh>

namespace mediascanner {
namespace qml {

class MediaFileWrapper : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString filename READ filename CONSTANT)
    Q_PROPERTY(QString uri READ uri CONSTANT)
    Q_PROPERTY(QString contentType READ contentType CONSTANT)
    Q_PROPERTY(QString eTag READ eTag CONSTANT)
    Q_PROPERTY(QString title READ title CONSTANT)
    Q_PROPERTY(QString author READ author CONSTANT)
    Q_PROPERTY(QString album READ album CONSTANT)
    Q_PROPERTY(QString albumArtist READ albumArtist CONSTANT)
    Q_PROPERTY(QString date READ date CONSTANT)
    Q_PROPERTY(QString genre READ genre CONSTANT)
    Q_PROPERTY(int discNumber READ discNumber CONSTANT)
    Q_PROPERTY(int trackNumber READ trackNumber CONSTANT)
    Q_PROPERTY(int duration READ duration CONSTANT)
    Q_PROPERTY(int width READ width CONSTANT)
    Q_PROPERTY(int height READ height CONSTANT)
    Q_PROPERTY(double latitude READ latitude CONSTANT)
    Q_PROPERTY(double longitude READ longitude CONSTANT)
    Q_PROPERTY(bool hasThumbnail READ hasThumbnail CONSTANT)
    Q_PROPERTY(QString art READ art CONSTANT)

public:
    MediaFileWrapper(const mediascanner::MediaFile &media, QObject *parent=0);
    QString filename() const;
    QString uri() const;
    QString contentType() const;
    QString eTag() const;
    QString title() const;
    QString author() const;
    QString album() const;
    QString albumArtist() const;
    QString date() const;
    QString genre() const;
    int discNumber() const;
    int trackNumber() const;
    int duration() const;
    int width() const;
    int height() const;
    double latitude() const;
    double longitude() const;
    bool hasThumbnail() const;
    QString art() const;

private:
    const mediascanner::MediaFile media;
};

}
}

#endif
