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

#include "plugin.hh"
#include "MediaFileWrapper.hh"
#include "MediaStoreWrapper.hh"
#include "AlbumsModel.hh"
#include "ArtistsModel.hh"
#include "GenresModel.hh"
#include "SongsModel.hh"
#include "SongsSearchModel.hh"

using namespace mediascanner::qml;

void MediaScannerPlugin::registerTypes(const char *uri) {
    qmlRegisterType<MediaStoreWrapper>(uri, 0, 1, "MediaStore");
    qmlRegisterUncreatableType<MediaFileWrapper>(uri, 0, 1, "MediaFile",
        "Use a MediaStore to retrieve MediaFiles");
    qmlRegisterType<AlbumsModel>(uri, 0, 1, "AlbumsModel");
    qmlRegisterType<ArtistsModel>(uri, 0, 1, "ArtistsModel");
    qmlRegisterType<GenresModel>(uri, 0, 1, "GenresModel");
    qmlRegisterType<SongsModel>(uri, 0, 1, "SongsModel");
    qmlRegisterType<SongsSearchModel>(uri, 0, 1, "SongsSearchModel");
}
