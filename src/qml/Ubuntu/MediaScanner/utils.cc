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

#include "utils.hh"
#include <QUrl>

namespace mediascanner {
namespace qml {

QString make_album_art_uri(const std::string &artist, const std::string &album) {
    QString result = "image://albumart/artist=";
    result += QUrl::toPercentEncoding(QString::fromStdString(artist));
    result += "&album=";
    result += QUrl::toPercentEncoding(QString::fromStdString(album));
    return result;
}

}
}
