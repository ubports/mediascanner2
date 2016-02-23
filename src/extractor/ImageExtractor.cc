/*
 * Copyright (C) 2013-2014 Canonical, Ltd.
 *
 * Authors:
 *    Jussi Pakkanen <jussi.pakkanen@canonical.com>
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

#include "ImageExtractor.hh"
#include "DetectedFile.hh"
#include "../mediascanner/MediaFile.hh"
#include "../mediascanner/MediaFileBuilder.hh"

#include <exif-loader.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include <ctime>
#include <memory>
#include <string>
#include <stdexcept>
#include <vector>

#include <unistd.h>

using namespace std;
using mediascanner::MediaFileBuilder;

namespace {
const char exif_date_template[] = "%Y:%m:%d %H:%M:%S";
const char iso8601_date_format[] = "%Y-%m-%dT%H:%M:%S";
const char iso8601_date_with_zone_format[] = "%Y-%m-%dT%H:%M:%S%z";

void parse_exif_date(ExifData *data, ExifByteOrder order, MediaFileBuilder &mfb) {
    static const std::vector<ExifTag> date_tags{
        EXIF_TAG_DATE_TIME_ORIGINAL,
        EXIF_TAG_DATE_TIME_DIGITIZED,
        EXIF_TAG_DATE_TIME,
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

int get_exif_int(ExifEntry *ent, ExifByteOrder order) {
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

void parse_exif_dimensions(ExifData *data, ExifByteOrder order, MediaFileBuilder &mfb) {
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

bool rational_to_degrees(ExifEntry *ent, ExifByteOrder order, double *out) {
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

void parse_exif_location(ExifData *data, ExifByteOrder order, MediaFileBuilder &mfb) {
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

}

namespace mediascanner {

bool ImageExtractor::extract_exif(const DetectedFile &d, MediaFileBuilder &mfb) {
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

void ImageExtractor::extract_pixbuf(const DetectedFile &d, MediaFileBuilder &builder) {
    gint width, height;

    if(!gdk_pixbuf_get_file_info(d.filename.c_str(), &width, &height)) {
        string msg("Could not determine resolution of ");
        msg += d.filename;
        msg += ".";
        throw runtime_error(msg);
    }
    builder.setWidth(width);
    builder.setHeight(height);

    if (d.mtime != 0) {
        auto t = static_cast<time_t>(d.mtime);
        char buf[1024];
        struct tm ptm;
        localtime_r(&t, &ptm);
        if (strftime(buf, sizeof(buf), iso8601_date_format, &ptm) != 0) {
            builder.setDate(buf);
        }
    }
}

void ImageExtractor::extract(const DetectedFile &d, MediaFileBuilder &builder) {
    if (!extract_exif(d, builder)) {
        extract_pixbuf(d, builder);
    }
    return;
}

}
