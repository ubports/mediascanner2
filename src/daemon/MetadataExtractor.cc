/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * Authors:
 *    Jussi Pakkanen <jussi.pakkanen@canonical.com>
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

#include "MetadataExtractor.hh"
#include "../mediascanner/MediaFile.hh"
#include "../mediascanner/MediaFileBuilder.hh"
#include "../mediascanner/internal/utils.hh"

#include <exif-loader.h>
#include <glib-object.h>
#include <gio/gio.h>
#include <gst/gst.h>
#include <gst/pbutils/pbutils.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include <cstdio>
#include <ctime>
#include <memory>
#include <string>
#include <stdexcept>
#include <vector>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

using namespace std;

namespace {
const char exif_date_template[] = "%Y:%m:%d %H:%M:%S";
const char iso8601_date_format[] = "%Y-%m-%dT%H:%M:%S";
const char iso8601_date_with_zone_format[] = "%Y-%m-%dT%H:%M:%S%z";
}

namespace mediascanner {

struct MetadataExtractorPrivate {
    std::unique_ptr<GstDiscoverer, decltype(&g_object_unref)> discoverer;
    MetadataExtractorPrivate() : discoverer(nullptr, g_object_unref) {};

    void extract_gst(const DetectedFile &d, MediaFileBuilder &mfb);
    bool extract_exif(const DetectedFile &d, MediaFileBuilder &mfb);
    void extract_pixbuf(const DetectedFile &d, MediaFileBuilder &mfb);
};

MetadataExtractor::MetadataExtractor(int seconds) {
    p = new MetadataExtractorPrivate();
    GError *error = nullptr;

    p->discoverer.reset(gst_discoverer_new(GST_SECOND * seconds, &error));
    if (not p->discoverer) {
        string errortxt(error->message);
        g_error_free(error);
        delete(p);

        string msg = "Failed to create discoverer: ";
        msg += errortxt;
        throw runtime_error(msg);
    }
    if (error) {
        // Sometimes this is filled in even though no error happens.
        g_error_free(error);
    }
}

MetadataExtractor::~MetadataExtractor() {
    delete p;
}

DetectedFile MetadataExtractor::detect(const std::string &filename) {
    std::unique_ptr<GFile, void(*)(void *)> file(
        g_file_new_for_path(filename.c_str()), g_object_unref);
    if (!file) {
        throw runtime_error("Could not create file object");
    }

    GError *error = nullptr;
    std::unique_ptr<GFileInfo, void(*)(void *)> info(
        g_file_query_info(
            file.get(),
            G_FILE_ATTRIBUTE_STANDARD_FAST_CONTENT_TYPE ","
            G_FILE_ATTRIBUTE_ETAG_VALUE,
            G_FILE_QUERY_INFO_NONE, /* cancellable */ nullptr, &error),
        g_object_unref);
    if (!info) {
        string errortxt(error->message);
        g_error_free(error);

        string msg("Query of file info for ");
        msg += filename;
        msg += " failed: ";
        msg += errortxt;
        throw runtime_error(msg);
    }

    string etag(g_file_info_get_etag(info.get()));
    string content_type(g_file_info_get_attribute_string(
        info.get(), G_FILE_ATTRIBUTE_STANDARD_FAST_CONTENT_TYPE));
    if (content_type.empty()) {
        throw runtime_error("Could not determine content type.");
    }

    MediaType type;
    if (content_type.find("audio/") == 0) {
        type = AudioMedia;
    } else if (content_type.find("video/") == 0) {
        type = VideoMedia;
    } else if (content_type.find("image/") == 0) {
        type = ImageMedia;
    } else {
        throw runtime_error(string("File ") + filename + " is not audio or video");
    }
    return DetectedFile(filename, etag, content_type, type);
}

static void
extract_tag_info (const GstTagList * list, const gchar * tag, gpointer user_data) {
    MediaFileBuilder *mfb = (MediaFileBuilder *) user_data;
    int i, num;
    string tagname(tag);

    if(tagname == GST_TAG_IMAGE) {
        mfb->setHasThumbnail(true);
        return;
    }
    num = gst_tag_list_get_tag_size (list, tag);
    for (i = 0; i < num; ++i) {
        const GValue *val;

        val = gst_tag_list_get_value_index (list, tag, i);
        if (G_VALUE_HOLDS_STRING(val)) {
            if (tagname == GST_TAG_ARTIST)
                mfb->setAuthor(g_value_get_string(val));
            else if (tagname == GST_TAG_TITLE)
                mfb->setTitle(g_value_get_string(val));
            else if (tagname == GST_TAG_ALBUM)
                mfb->setAlbum(g_value_get_string(val));
            else if (tagname == GST_TAG_ALBUM_ARTIST)
                mfb->setAlbumArtist(g_value_get_string(val));
            else if (tagname == GST_TAG_GENRE)
                mfb->setGenre(g_value_get_string(val));
        } else if (G_VALUE_HOLDS(val, GST_TYPE_DATE_TIME)) {
            if (tagname == GST_TAG_DATE_TIME) {
                GstDateTime *dt = static_cast<GstDateTime*>(g_value_get_boxed(val));
                char *dt_string = gst_date_time_to_iso8601_string(dt);
                mfb->setDate(dt_string);
                g_free(dt_string);
            }
        } else if (G_VALUE_HOLDS_UINT(val)) {
            if (tagname == GST_TAG_TRACK_NUMBER) {
                mfb->setTrackNumber(g_value_get_uint(val));
            } else if (tagname == GST_TAG_ALBUM_VOLUME_NUMBER) {
                mfb->setDiscNumber(g_value_get_uint(val));
            }
        }

    }
}

void MetadataExtractorPrivate::extract_gst(const DetectedFile &d, MediaFileBuilder &mfb) {
    string uri = getUri(d.filename);

    GError *error = nullptr;
    unique_ptr<GstDiscovererInfo, void(*)(void *)> info(
        gst_discoverer_discover_uri(discoverer.get(), uri.c_str(), &error),
        g_object_unref);
    if (info.get() == nullptr) {
        string errortxt(error->message);
        g_error_free(error);

        string msg = "Discovery of file ";
        msg += d.filename;
        msg += " failed: ";
        msg += errortxt;
        throw runtime_error(msg);
    }
    if (error) {
        // Sometimes this gets filled in even if no error actually occurs.
        g_error_free(error);
        error = nullptr;
    }

    if (gst_discoverer_info_get_result(info.get()) != GST_DISCOVERER_OK) {
        throw runtime_error("Unable to discover file " + d.filename);
    }

    const GstTagList *tags = gst_discoverer_info_get_tags(info.get());
    if (tags != nullptr) {
        gst_tag_list_foreach(tags, extract_tag_info, &mfb);
    }
    mfb.setDuration(static_cast<int>(
        gst_discoverer_info_get_duration(info.get())/GST_SECOND));

    /* Check for video specific information */
    unique_ptr<GList, void(*)(GList*)> streams(
        gst_discoverer_info_get_stream_list(info.get()),
        gst_discoverer_stream_info_list_free);
    for (const GList *l = streams.get(); l != nullptr; l = l->next) {
        auto stream_info = static_cast<GstDiscovererStreamInfo*>(l->data);

        if (GST_IS_DISCOVERER_VIDEO_INFO(stream_info)) {
            mfb.setWidth(gst_discoverer_video_info_get_width(
                             GST_DISCOVERER_VIDEO_INFO(stream_info)));
            mfb.setHeight(gst_discoverer_video_info_get_height(
                              GST_DISCOVERER_VIDEO_INFO(stream_info)));
            break;
        }
    }
}

static void parse_exif_date(ExifData *data, ExifByteOrder order, MediaFileBuilder &mfb) {
    static const std::vector<ExifTag> date_tags{
        EXIF_TAG_DATE_TIME,
        EXIF_TAG_DATE_TIME_ORIGINAL,
        EXIF_TAG_DATE_TIME_DIGITIZED
    };
    struct tm timeinfo;
    bool have_date = false;

    for (ExifTag tag : date_tags) {
        ExifEntry *ent = exif_data_get_entry(data, tag);
        if (ent == nullptr) {
            continue;
        }
        if (strptime((const char*)ent->data, exif_date_template, &timeinfo) != nullptr) {
            have_date = true;
            break;
        }
    }
    if (!have_date) {
        return;
    }

    char buf[100];
    ExifEntry *ent = exif_data_get_entry(data, EXIF_TAG_TIME_ZONE_OFFSET);
    if (ent) {
        timeinfo.tm_gmtoff = (int)exif_get_sshort(ent->data, order) * 3600;

        if (strftime(buf, sizeof(buf), iso8601_date_with_zone_format, &timeinfo) != 0) {
            mfb.setDate(buf);
        }
    } else {
        /* No time zone info */
        if (strftime(buf, sizeof(buf), iso8601_date_format, &timeinfo) != 0) {
            mfb.setDate(buf);
        }
    }
}

static int get_exif_int(ExifEntry *ent, ExifByteOrder order) {
    switch (ent->format) {
    case EXIF_FORMAT_BYTE:
        return (unsigned char)ent->data[0];
    case EXIF_FORMAT_SHORT:
        return exif_get_short(ent->data, order);
    case EXIF_FORMAT_LONG:
        return exif_get_long(ent->data, order);
    case EXIF_FORMAT_SBYTE:
        return (signed char)ent->data[0];
    case EXIF_FORMAT_SSHORT:
        return exif_get_sshort(ent->data, order);
    case EXIF_FORMAT_SLONG:
        return exif_get_slong(ent->data, order);
    default:
        break;
    }
    return 0;
}

static void parse_exif_dimensions(ExifData *data, ExifByteOrder order, MediaFileBuilder &mfb) {
    ExifEntry *w_ent = exif_data_get_entry(data, EXIF_TAG_PIXEL_X_DIMENSION);
    ExifEntry *h_ent = exif_data_get_entry(data, EXIF_TAG_PIXEL_Y_DIMENSION);
    ExifEntry *o_ent = exif_data_get_entry(data, EXIF_TAG_ORIENTATION);

    if (!w_ent || !h_ent) {
        return;
    }
    int width = get_exif_int(w_ent, order);
    int height = get_exif_int(h_ent, order);

    // Optionally swap height and width depending on orientation
    if (o_ent) {
        int tmp;

        // exif_data_fix() has ensured this is a short.
        switch (exif_get_short(o_ent->data, order)) {
        case 5: // Mirror horizontal and rotate 270 CW
        case 6: // Rotate 90 CW
        case 7: // Mirror horizontal and rotate 90 CW
        case 8: // Rotate 270 CW
            tmp = width;
            width = height;
            height = tmp;
            break;
        default:
            break;
        }
    }
    mfb.setWidth(width);
    mfb.setHeight(height);
}

static bool rational_to_degrees(ExifEntry *ent, ExifByteOrder order, double *out) {
    if (ent->format != EXIF_FORMAT_RATIONAL) {
        return false;
    }

    ExifRational r = exif_get_rational(ent->data, order);
    *out = ((double) r.numerator) / r.denominator;

    // Minutes
    if (ent->components >= 2) {
        r = exif_get_rational(ent->data + exif_format_get_size(EXIF_FORMAT_RATIONAL), order);
        *out += ((double) r.numerator) / r.denominator / 60;
    }
    // Seconds
    if (ent->components >= 3) {
        r = exif_get_rational(ent->data + 2 * exif_format_get_size(EXIF_FORMAT_RATIONAL), order);
        *out += ((double) r.numerator) / r.denominator / 3600;
    }
    return true;
}

static void parse_exif_location(ExifData *data, ExifByteOrder order, MediaFileBuilder &mfb) {
    ExifContent *ifd = data->ifd[EXIF_IFD_GPS];
    ExifEntry *lat_ent = exif_content_get_entry(ifd, (ExifTag)EXIF_TAG_GPS_LATITUDE);
    ExifEntry *latref_ent = exif_content_get_entry(ifd, (ExifTag)EXIF_TAG_GPS_LATITUDE_REF);
    ExifEntry *long_ent = exif_content_get_entry(ifd, (ExifTag)EXIF_TAG_GPS_LONGITUDE);
    ExifEntry *longref_ent = exif_content_get_entry(ifd, (ExifTag)EXIF_TAG_GPS_LONGITUDE_REF);

    if (!lat_ent || !long_ent) {
        return;
    }

    double latitude, longitude;
    if (!rational_to_degrees(lat_ent, order, &latitude)) {
        return;
    }
    if (!rational_to_degrees(long_ent, order, &longitude)) {
        return;
    }
    if (latref_ent && latref_ent->data[0] == 'S') {
        latitude = -latitude;
    }
    if (longref_ent && longref_ent->data[0] == 'W') {
        longitude = -longitude;
    }
    mfb.setLatitude(latitude);
    mfb.setLongitude(longitude);
}

bool MetadataExtractorPrivate::extract_exif(const DetectedFile &d, MediaFileBuilder &mfb) {
    std::unique_ptr<ExifLoader, void(*)(ExifLoader*)> loader(
        exif_loader_new(), exif_loader_unref);
    exif_loader_write_file(loader.get(), d.filename.c_str());

    std::unique_ptr<ExifData, void(*)(ExifData*)> data(
        exif_loader_get_data(loader.get()), exif_data_unref);
    loader.reset();

    if (!data) {
        return false;
    }
    exif_data_fix(data.get());
    ExifByteOrder order = exif_data_get_byte_order(data.get());

    parse_exif_date(data.get(), order, mfb);
    parse_exif_dimensions(data.get(), order, mfb);
    parse_exif_location(data.get(), order, mfb);
    return true;
}

void MetadataExtractorPrivate::extract_pixbuf(const DetectedFile &d, MediaFileBuilder &mfb) {
    gint width, height;

    if(!gdk_pixbuf_get_file_info(d.filename.c_str(), &width, &height)) {
        string msg("Could not determine resolution of ");
        msg += d.filename;
        msg += ".";
        throw runtime_error(msg);
    }

    struct stat info;
    if(stat(d.filename.c_str(), &info) == 0) {
        char buf[1024];
        struct tm ptm;
        localtime_r(&info.st_mtime, &ptm);
        if (strftime(buf, sizeof(buf), iso8601_date_format, &ptm) != 0) {
            mfb.setDate(buf);
        }
    }


    mfb.setWidth(width);
    mfb.setHeight(height);
}

MediaFile MetadataExtractor::extract(const DetectedFile &d) {
    printf("Extracting metadata from %s.\n", d.filename.c_str());
    MediaFileBuilder mfb(d.filename);
    mfb.setETag(d.etag);
    mfb.setContentType(d.content_type);
    mfb.setType(d.type);

    switch (d.type) {
    case ImageMedia:
        if(!p->extract_exif(d, mfb)) {
            p->extract_pixbuf(d, mfb);
        }
        break;
    default:
        p->extract_gst(d, mfb);
        break;
    }

    return mfb;
}

MediaFile MetadataExtractor::fallback_extract(const DetectedFile &d) {
    return MediaFileBuilder(d.filename).setType(d.type);
}

}
