/*
 * Copyright (C) 2016 Canonical, Ltd.
 *
 * Authors:
 *    James Henstridge <james.henstridge@canonical.com>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of version 3 of the GNU General Public License as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "TaglibExtractor.hh"
#include "DetectedFile.hh"
#include "../mediascanner/MediaFile.hh"
#include "../mediascanner/MediaFileBuilder.hh"

#include <glib.h>
#include <gio/gio.h>
#include <gst/gstdatetime.h>

#include <taglib/flacfile.h>
#include <taglib/id3v1tag.h>
#include <taglib/id3v2framefactory.h>
#include <taglib/id3v2tag.h>
#include <taglib/mp4file.h>
#include <taglib/mpegfile.h>
#include <taglib/oggflacfile.h>
#include <taglib/opusfile.h>
#include <taglib/tfilestream.h>
#include <taglib/vorbisfile.h>

#include <cassert>
#include <cstdio>
#include <memory>
#include <string>
#include <stdexcept>

using namespace std;
using mediascanner::MediaFileBuilder;

namespace {

string get_content_type(const string &filename) {
    unique_ptr<GFile, decltype(&g_object_unref)> file(
        g_file_new_for_path(filename.c_str()), g_object_unref);
    assert(file);

    GError *error = nullptr;
    unique_ptr<GFileInfo, decltype(&g_object_unref)> info(
        g_file_query_info(
            file.get(),
            G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE,
            G_FILE_QUERY_INFO_NONE, nullptr, &error),
        g_object_unref);
    if (!info) {
        fprintf(stderr, "Failed to detect content type of '%s': %s\n",
                filename.c_str(), error->message);
        g_error_free(error);
        return string();
    }
    return g_file_info_get_content_type(info.get());
}

string check_date(const string &date_string) {
    // Parse and reserialise the date using GstDateTime to check its
    // consistency.
    unique_ptr<GstDateTime, decltype(&gst_date_time_unref)> dt(
        gst_date_time_new_from_iso8601_string(date_string.c_str()),
        gst_date_time_unref);
    if (!dt) {
        return string();
    }
    char *iso = gst_date_time_to_iso8601_string(dt.get());
    if (!iso) {
        return string();
    }
    string result(iso);
    g_free(iso);
    return result;
}

void parse_common(const TagLib::File &file, MediaFileBuilder &builder) {
    TagLib::Tag *tag = file.tag();

    const TagLib::AudioProperties *audio_props = file.audioProperties();
    if (audio_props) {
        builder.setDuration(audio_props->length());
    }

    TagLib::String s = tag->title();
    if (!s.isEmpty()) {
        builder.setTitle(s.to8Bit(true));
    }
    s = tag->artist();
    if (!s.isEmpty()) {
        builder.setAuthor(s.to8Bit(true));
    }
    s = tag->album();
    if (!s.isEmpty()) {
        builder.setAlbum(s.to8Bit(true));
    }
    s = tag->genre();
    if (!s.isEmpty()) {
        builder.setGenre(s.to8Bit(true));
    }
    unsigned int year = tag->year();
    if (year > 0 && year < 0xffff) {
        GDate date;
        g_date_clear(&date, 1);
        g_date_set_dmy(&date, 1, G_DATE_JANUARY, year);
        if (g_date_valid(&date)) {
            char buf[100] = { 0, };
            g_date_strftime(buf, sizeof(buf), "%Y", &date);
            builder.setDate(buf);
        }
    }
    unsigned int track = tag->track();
    if (track != 0) {
        builder.setTrackNumber(track);
    }
}

void parse_xiph_comment(const TagLib::Ogg::XiphComment *tag, MediaFileBuilder &builder) {
    const auto& fields = tag->fieldListMap();

    if (!fields["DATE"].isEmpty()) {
        builder.setDate(check_date(fields["DATE"].front().to8Bit(true)));
    }

    if (!fields["ALBUMARTIST"].isEmpty()) {
        builder.setAlbumArtist(fields["ALBUMARTIST"].front().to8Bit(true));
    }

    if (!fields["DISCNUMBER"].isEmpty()) {
        builder.setDiscNumber(fields["DISCNUMBER"].front().toInt());
    }

    if (!fields["COVERART"].isEmpty() || !fields["METADATA_BLOCK_PICTURE"].isEmpty()) {
        builder.setHasThumbnail(true);
    }
}

void parse_id3v2(const TagLib::ID3v2::Tag *tag, MediaFileBuilder &builder) {
    const auto& frames = tag->frameListMap();

    if (!frames["TDRC"].isEmpty()) {
        // ID3v2.4
        builder.setDate(check_date(frames["TDRC"].front()->toString().to8Bit(true)));
    } else if (!frames["TYER"].isEmpty()) {
        // ID3v2.3 and below
        int year = frames["TYER"].front()->toString().toInt();
        GDate date;
        g_date_clear(&date, 1);
        if (year > 0 && year < 0xffff) {
            g_date_set_year(&date, year);
        }
        if (!frames["TDAT"].isEmpty()) {
            int ddmm = frames["TDAT"].front()->toString().toInt();
            g_date_set_month(&date, static_cast<GDateMonth>(ddmm % 100));
            g_date_set_day(&date, ddmm / 100);
        }
        if (g_date_valid(&date)) {
            char buf[100] = { 0, };
            g_date_strftime(buf, sizeof(buf), "%Y", &date);
            builder.setDate(buf);
        }
    }

    if (!frames["TPE2"].isEmpty()) {
        builder.setAlbumArtist(frames["TPE2"].front()->toString().to8Bit(true));
    }

    if (!frames["TPOS"].isEmpty()) {
        builder.setDiscNumber(frames["TPOS"].front()->toString().toInt());
    }

    if (!frames["APIC"].isEmpty()) {
        builder.setHasThumbnail(true);
    }
}

void parse_mp4(const TagLib::MP4::Tag *tag, MediaFileBuilder &builder) {
    const auto& items = const_cast<TagLib::MP4::Tag*>(tag)->itemListMap();
    if (items.contains("aART")) {
        builder.setAlbumArtist(items["aART"].toStringList().front().to8Bit(true));
    }

    if (items.contains("disk")) {
        builder.setDiscNumber(items["disk"].toIntPair().first);
    }

    if (items.contains("covr")) {
        builder.setHasThumbnail(true);
    }
}

}

namespace mediascanner {

bool TaglibExtractor::extract(const DetectedFile &d, MediaFileBuilder &builder) {
    string content_type = get_content_type(d.filename);
    if (content_type.empty()) {
        return false;
    }

    unique_ptr<TagLib::FileStream> fs(new TagLib::FileStream(d.filename.c_str(), true));
    if (!fs->isOpen()) {
        return false;
    }

    if (content_type == "audio/x-vorbis+ogg") {
        TagLib::Ogg::Vorbis::File file(fs.get());
        parse_common(file, builder);
        parse_xiph_comment(file.tag(), builder);
    } else if (content_type == "audio/x-opus+ogg") {
        TagLib::Ogg::Opus::File file(fs.get());
        parse_common(file, builder);
        parse_xiph_comment(file.tag(), builder);
    } else if (content_type == "audio/x-flac+ogg") {
        TagLib::Ogg::FLAC::File file(fs.get());
        parse_common(file, builder);
        parse_xiph_comment(file.tag(), builder);
    } else if (content_type == "audio/flac") {
        TagLib::FLAC::File file(fs.get(), TagLib::ID3v2::FrameFactory::instance());
        parse_common(file, builder);
        if (file.hasID3v2Tag()) {
            parse_id3v2(file.ID3v2Tag(), builder);
        }
        if (file.hasXiphComment()) {
            parse_xiph_comment(file.xiphComment(), builder);
        }
    } else if (content_type == "audio/mpeg") {
        TagLib::MPEG::File file(fs.get(), TagLib::ID3v2::FrameFactory::instance());
        parse_common(file, builder);
        if (file.hasID3v2Tag()) {
            parse_id3v2(file.ID3v2Tag(), builder);
        }
    } else if (content_type == "audio/mp4") {
        TagLib::MP4::File file(fs.get());
        parse_common(file, builder);
        parse_mp4(file.tag(), builder);
    } else {
        return false;
    }
    return true;
}

}
