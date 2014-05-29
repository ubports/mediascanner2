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
#include <cstdint>
#include <string>

#include <core/dbus/object.h>

#include <mediascanner/MediaFile.hh>
#include <mediascanner/MediaFileBuilder.hh>
#include <mediascanner/Album.hh>
#include "dbus-codec.hh"

using core::dbus::Message;
using core::dbus::Codec;
using mediascanner::MediaFile;
using mediascanner::MediaFileBuilder;
using mediascanner::MediaType;
using mediascanner::Album;
using std::string;

void Codec<MediaFile>::encode_argument(Message::Writer &out, const MediaFile &file) {
    auto w = out.open_structure();
    core::dbus::encode_argument(w, file.getFileName());
    core::dbus::encode_argument(w, file.getContentType());
    core::dbus::encode_argument(w, file.getETag());
    core::dbus::encode_argument(w, file.getTitle());
    core::dbus::encode_argument(w, file.getAuthor());
    core::dbus::encode_argument(w, file.getAlbum());
    core::dbus::encode_argument(w, file.getAlbumArtist());
    core::dbus::encode_argument(w, file.getDate());
    core::dbus::encode_argument(w, file.getGenre());
    core::dbus::encode_argument(w, (int32_t)file.getDiscNumber());
    core::dbus::encode_argument(w, (int32_t)file.getTrackNumber());
    core::dbus::encode_argument(w, (int32_t)file.getDuration());
    core::dbus::encode_argument(w, (int32_t)file.getType());
    out.close_structure(std::move(w));
}

void Codec<MediaFile>::decode_argument(Message::Reader &in, MediaFile &file) {
    auto r = in.pop_structure();
    string filename, content_type, etag, title, author;
    string album, album_artist, date, genre;
    int32_t disc_number, track_number, duration, type;
    r >> filename >> content_type >> etag >> title >> author
      >> album >> album_artist >> date >> genre
      >> disc_number >> track_number >> duration >> type;
    file = MediaFileBuilder(filename)
        .setContentType(content_type)
        .setETag(etag)
        .setTitle(title)
        .setAuthor(author)
        .setAlbum(album)
        .setAlbumArtist(album_artist)
        .setDate(date)
        .setGenre(genre)
        .setDiscNumber(disc_number)
        .setTrackNumber(track_number)
        .setDuration(duration)
        .setType((MediaType)type);
}

void Codec<Album>::encode_argument(Message::Writer &out, const Album &album) {
    auto w = out.open_structure();
    core::dbus::encode_argument(w, album.getTitle());
    core::dbus::encode_argument(w, album.getArtist());
    out.close_structure(std::move(w));
}

void Codec<Album>::decode_argument(Message::Reader &in, Album &album) {
    auto r = in.pop_structure();
    string title, artist;
    r >> title >> artist;
    album = Album(title, artist);
}
