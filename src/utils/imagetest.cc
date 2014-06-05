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

#include <csetjmp>
#include <cstdio>
#include <ctime>
#include <exception>
#include <memory>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <jpeglib.h>
#include <jerror.h>
#include <exif-loader.h>

struct ErrorData {
    struct jpeg_error_mgr jerr;
    sigjmp_buf env;
    std::exception_ptr exc;
};

static void error_exit_handler(j_common_ptr cinfo) noexcept {
    ErrorData *mgr = reinterpret_cast<ErrorData*>(cinfo->err);

    char buffer[JMSG_LENGTH_MAX];
    cinfo->err->format_message(cinfo, buffer);
    mgr->exc = std::make_exception_ptr(std::runtime_error(buffer));

    siglongjmp(mgr->env, 1);
}

static void output_message_handler(j_common_ptr) noexcept {
    /* do nothing */
}

static void print_jpeg(const char *infile) {
    FILE *fp;
    if ((fp = fopen(infile, "rb")) == NULL) {
        fprintf(stderr, "Could not open %s\n", infile);
        return;
    }

    struct jpeg_decompress_struct cinfo;
    ErrorData mgr;
    cinfo.err = jpeg_std_error(&mgr.jerr);
    mgr.jerr.error_exit = error_exit_handler;
    mgr.jerr.output_message = output_message_handler;

    // No objects with destructors can should be created after this call.
    if (sigsetjmp(mgr.env, 1)) {
        jpeg_destroy_decompress(&cinfo);
        std::rethrow_exception(mgr.exc);
    }

    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, fp);
    jpeg_read_header(&cinfo, true);

    std::cout << "Image size (" << cinfo.image_width << ", " << cinfo.image_height << ")" << std::endl;

    jpeg_destroy_decompress(&cinfo);
}

static bool get_dimensions(ExifData *ed, ExifByteOrder order, unsigned long *width, unsigned long *height) {
    ExifEntry *w_ent = exif_data_get_entry(ed, EXIF_TAG_PIXEL_X_DIMENSION);
    ExifEntry *h_ent = exif_data_get_entry(ed, EXIF_TAG_PIXEL_Y_DIMENSION);
    ExifEntry *o_ent = exif_data_get_entry(ed, EXIF_TAG_ORIENTATION);

    if (!w_ent || !h_ent) {
        return false;
    }

    switch (w_ent->format) {
    case EXIF_FORMAT_SHORT:
        *width = exif_get_short(w_ent->data, order);
        break;
    case EXIF_FORMAT_SSHORT:
        *width = exif_get_sshort(w_ent->data, order);
        break;
    case EXIF_FORMAT_LONG:
        *width = exif_get_long(w_ent->data, order);
        break;
    case EXIF_FORMAT_SLONG:
        *width = exif_get_slong(w_ent->data, order);
        break;
    default:
        return false;
    }

    switch (h_ent->format) {
    case EXIF_FORMAT_SHORT:
        *height = exif_get_short(h_ent->data, order);
        break;
    case EXIF_FORMAT_SSHORT:
        *height = exif_get_sshort(h_ent->data, order);
        break;
    case EXIF_FORMAT_LONG:
        *height = exif_get_long(h_ent->data, order);
        break;
    case EXIF_FORMAT_SLONG:
        *height = exif_get_slong(h_ent->data, order);
        break;
    default:
        return false;
    }

    // Check if the image has been rotated.
    if (o_ent) {
        unsigned long tmp;

        // exif_data_fix() will have converted this field to a short,
        // so no need to check the type.
        switch (exif_get_short(o_ent->data, order)) {
        case 5: // Mirror horizontal and rotate 270 CW
        case 6: // Rotate 90 CW
        case 7: // Mirror horizontal and rotate 90 CW
        case 8: // Rotate 270 CW
            tmp = *width;
            *width = *height;
            *height = tmp;
            break;
        default:
            break;
        }
    }
    return true;
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

static double get_gps(ExifData *ed, ExifByteOrder order, double *lat, double *long_) {
    ExifContent *ifd = ed->ifd[EXIF_IFD_GPS];
    ExifEntry *lat_ent = exif_content_get_entry(ifd, (ExifTag)EXIF_TAG_GPS_LATITUDE);
    ExifEntry *latref_ent = exif_content_get_entry(ifd, (ExifTag)EXIF_TAG_GPS_LATITUDE_REF);
    ExifEntry *long_ent = exif_content_get_entry(ifd, (ExifTag)EXIF_TAG_GPS_LONGITUDE);
    ExifEntry *longref_ent = exif_content_get_entry(ifd, (ExifTag)EXIF_TAG_GPS_LONGITUDE_REF);

    if (!lat_ent || !long_ent) {
        return false;
    }

    if (!rational_to_degrees(lat_ent, order, lat)) {
        return false;
    }
    if (!rational_to_degrees(long_ent, order, long_)) {
        return false;
    }

    if (latref_ent && latref_ent->data[0] == 'S') {
        *lat = -(*lat);
    }
    if (longref_ent && longref_ent->data[0] == 'W') {
        *long_ = -(*long_);
    }
    return true;
}

static std::string parse_date(ExifData *ed, ExifByteOrder order) {
    static const char exif_date_template[] = "%Y:%m:%d %H:%M:%S";
    struct tm timeinfo;
    bool have_date = false;

    const std::vector<ExifTag> date_tags{
        EXIF_TAG_DATE_TIME,
            EXIF_TAG_DATE_TIME_ORIGINAL,
            EXIF_TAG_DATE_TIME_DIGITIZED
            };

    for (ExifTag tag : date_tags) {
        ExifEntry *ent = exif_data_get_entry(ed, tag);
        if (ent == nullptr) {
            continue;
        }
        struct tm timeinfo;
        if (strptime((const char*)ent->data, exif_date_template, &timeinfo) != nullptr) {
            have_date = true;
            break;
        }
    }

    if (!have_date) {
        return std::string();
    }

    char buf[100];
    ExifEntry *ent = exif_data_get_entry(ed, EXIF_TAG_TIME_ZONE_OFFSET);
    if (ent) {
        timeinfo.tm_gmtoff = (int)exif_get_sshort(ent->data, order) * 3600;

        if (strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S%z", &timeinfo) == 0) {
            return std::string();
        }
    } else {
        /* No time zone info */
        if (strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", &timeinfo) == 0) {
            return std::string();
        }
    }
    return buf;
}

static void print_exif(const char *infile) {
    std::unique_ptr<ExifLoader, void(*)(ExifLoader*)> l(
        exif_loader_new(), exif_loader_unref);
    exif_loader_write_file(l.get(), infile);

    std::unique_ptr<ExifData, void(*)(ExifData*)> ed(
        exif_loader_get_data(l.get()), exif_data_unref);
    l.reset();

    if (!ed) {
        std::cout << "Could not read exif data." << std::endl;
        return;
    }

    exif_data_fix(ed.get());

    ExifByteOrder order = exif_data_get_byte_order(ed.get());
    unsigned long width, height;
    if (get_dimensions(ed.get(), order, &width, &height)) {
        std::cout << "Image size (" << width << ", " << height << ")" << std::endl;
    } else {
        std::cout << "Image size not in exif tags." << std::endl;
    }

    double latitude, longitude;
    if (get_gps(ed.get(), order, &latitude, &longitude)) {
        std::cout << "Image coordinates: (" << latitude << ", " << longitude << ")." << std::endl;
    } else {
        std::cout << "Image does not contain GPS latitude." << std::endl;
    }

    std::string date = parse_date(ed.get(), order);
    if (!date.empty()) {
        std::cout << "Image taken on " << date << std::endl;
    } else {
        std::cout << "Image does not contain time data." << std::endl;
    }
}

int main(int argc, char **argv) {
    if(argc != 2) {
        std::cout << argv[0] << "image_file" << std::endl;
        return 1;
    }
    try {
        print_jpeg(argv[1]);
    } catch (const std::exception &e) {
        fprintf(stderr, "Failed to read JPEG: %s\n", e.what());
    }
    print_exif(argv[1]);
    return 0;
}
