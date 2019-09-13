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

#include "dbus-codec.hh"
#include <cstdint>
#include <string>

#include <core/dbus/object.h>

#include <mediascanner/MediaFile.hh>
#include <mediascanner/MediaFileBuilder.hh>
#include <mediascanner/Album.hh>
#include <mediascanner/Filter.hh>

using core::dbus::Message;
using core::dbus::Codec;
using core::dbus::types::Variant;
using mediascanner::MediaFile;
using mediascanner::MediaFileBuilder;
using mediascanner::MediaOrder;
using mediascanner::MediaType;
using mediascanner::Album;
using mediascanner::Filter;
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
    core::dbus::encode_argument(w, (int32_t)file.getWidth());
    core::dbus::encode_argument(w, (int32_t)file.getHeight());
    core::dbus::encode_argument(w, file.getLatitude());
    core::dbus::encode_argument(w, file.getLongitude());
    core::dbus::encode_argument(w, file.getHasThumbnail());
    core::dbus::encode_argument(w, file.getModificationTime());
    core::dbus::encode_argument(w, (int32_t)file.getType());
    out.close_structure(std::move(w));
}

void Codec<MediaFile>::decode_argument(Message::Reader &in, MediaFile &file) {
    auto r = in.pop_structure();
    string filename, content_type, etag, title, author;
    string album, album_artist, date, genre;
    int32_t disc_number, track_number, duration, width, height, type;
    double latitude, longitude;
    bool has_thumbnail;
    uint64_t mtime;
    r >> filename >> content_type >> etag >> title >> author
      >> album >> album_artist >> date >> genre
      >> disc_number >> track_number >> duration
      >> width >> height >> latitude >> longitude >> has_thumbnail
      >> mtime >> type;
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
        .setWidth(width)
        .setHeight(height)
        .setLatitude(latitude)
        .setLongitude(longitude)
        .setHasThumbnail(has_thumbnail)
        .setModificationTime(mtime)
        .setType((MediaType)type);
}

void Codec<Album>::encode_argument(Message::Writer &out, const Album &album) {
    auto w = out.open_structure();
    core::dbus::encode_argument(w, album.getTitle());
    core::dbus::encode_argument(w, album.getArtist());
    core::dbus::encode_argument(w, album.getDate());
    core::dbus::encode_argument(w, album.getGenre());
    core::dbus::encode_argument(w, album.getArtFile());
    core::dbus::encode_argument(w, album.getHasThumbnail());
    core::dbus::encode_argument(w, album.getArtistCount());
    out.close_structure(std::move(w));
}

void Codec<Album>::decode_argument(Message::Reader &in, Album &album) {
    auto r = in.pop_structure();
    string title, artist, date, genre, art_file;
    bool has_thumbnail;
    int artist_count;
    r >> title >> artist >> date >> genre >> art_file >> has_thumbnail >> artist_count;

    album = Album(title, artist, date, genre, art_file, has_thumbnail, artist_count);
}

void Codec<Filter>::encode_argument(Message::Writer &out, const Filter &filter) {
    auto w = out.open_array(core::dbus::types::Signature("{sv}"));

    if (filter.hasArtist()) {
        w.close_dict_entry(
            w.open_dict_entry() << string("artist") << Variant::encode(filter.getArtist()));
    }
    if (filter.hasAlbum()) {
        w.close_dict_entry(
            w.open_dict_entry() << string("album") << Variant::encode(filter.getAlbum()));
    }
    if (filter.hasAlbumArtist()) {
        w.close_dict_entry(
            w.open_dict_entry() << string("album_artist") << Variant::encode(filter.getAlbumArtist()));
    }
    if (filter.hasGenre()) {
        w.close_dict_entry(
            w.open_dict_entry() << string("genre") << Variant::encode(filter.getGenre()));
    }

    w.close_dict_entry(
        w.open_dict_entry() << string("offset") << Variant::encode((int32_t)filter.getOffset()));
    w.close_dict_entry(
        w.open_dict_entry() << string("limit") << Variant::encode((int32_t)filter.getLimit()));
    w.close_dict_entry(
        w.open_dict_entry() << string("order") << Variant::encode(static_cast<int32_t>(filter.getOrder())));
    w.close_dict_entry(
        w.open_dict_entry() << string("reverse") << Variant::encode(filter.getReverse()));

    out.close_array(std::move(w));
}

void Codec<Filter>::decode_argument(Message::Reader &in, Filter &filter) {
    auto r = in.pop_array();

    filter.clear();
    while (r.type() != ArgumentType::invalid) {
        string key;
        Variant value;
        r.pop_dict_entry() >> key >> value;

        if (key == "artist") {
            filter.setArtist(value.as<string>());
        } else if (key == "album") {
            filter.setAlbum(value.as<string>());
        } else if (key == "album_artist") {
            filter.setAlbumArtist(value.as<string>());
        } else if (key == "genre") {
            filter.setGenre(value.as<string>());
        } else if (key == "offset") {
            filter.setOffset(value.as<int32_t>());
        } else if (key == "limit") {
            filter.setLimit(value.as<int32_t>());
        } else if (key == "order") {
            filter.setOrder(static_cast<MediaOrder>(value.as<int32_t>()));
        } else if (key == "reverse") {
            filter.setReverse(value.as<bool>());
        }
    }
}
