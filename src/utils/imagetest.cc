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
#include <iostream>
#include <stdexcept>

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

static void print_exif(const char *infile) {
    ExifLoader *l;
    l = exif_loader_new();
    ExifData *ed;
    exif_loader_write_file(l, infile);
    ed = exif_loader_get_data(l);
    exif_loader_unref(l);
    ExifByteOrder bo = exif_data_get_byte_order(ed);
    if(ed) {
        ExifEntry *e = exif_data_get_entry(ed, EXIF_TAG_IMAGE_WIDTH);
        if(!e) {
            std::cout << "Image size not in exif tags." << std::endl;
        } else {
            int w = exif_get_long(e->data, bo);
            e = exif_data_get_entry(ed, EXIF_TAG_IMAGE_LENGTH);
            int h = exif_get_long(e->data, exif_data_get_byte_order(ed));
            std::cout << "Image size (" << w << ", " << h << ")" << std::endl;
        }
        e = exif_data_get_entry(ed, (ExifTag)EXIF_TAG_GPS_LATITUDE);
        if(!e) {
            std::cout << "Image does not contain GPS latitude." << std::endl;
        } else {
            ExifRational latitude = exif_get_rational(e->data, bo);
            e = exif_data_get_entry(ed, (ExifTag)EXIF_TAG_GPS_LONGITUDE);
            if(!e) {
                std::cout << "Image does not contain GPS longitude." << std::endl;
            } else {
                ExifRational longitude = exif_get_rational(e->data, bo);
                double lat = ((double) latitude.numerator) / latitude.denominator;
                double lon = ((double) longitude.numerator) / longitude.denominator;
                std::cout << "Image coordinates: (" << lat << ", " << lon << ")." << std::endl;
            }
        }
        e = exif_data_get_entry(ed, EXIF_TAG_DATE_TIME);
        if(!e) {
            std::cout << "Image does not contain time data." << std::endl;
        } else {
            struct tm timeinfo;
            if(getdate_r((const char*)e->data, &timeinfo) != 0) {
                const char templ[] = "%Y:%m:%d %H:%M:%S";
                if(strptime((const char*)e->data, templ, &timeinfo) == nullptr) {
                    std::cout << "Could not parse exif date string "
                            << e->data << "." << std::endl;
                } else {
                    std::cout << "Image taken " << timeinfo.tm_year << "/" << timeinfo.tm_mon
                            << "/" << timeinfo.tm_mday << std::endl;
                }
            } else {
                std::cout << "Image taken " << timeinfo.tm_year << "/" << timeinfo.tm_mon
                        << "/" << timeinfo.tm_mday << std::endl;
            }
        }
    } else {
        std::cout << "Could not read exif data." << std::endl;
    }
    exif_data_unref(ed);
}

int main(int argc, char **argv) {
    if(argc != 2) {
        std::cout << argv[0] << "image_file" << std::endl;
        return 1;
    }
    try {
        print_jpeg(argv[1]);
    } catch (const std::exception &e) {
        fprintf(stderr, "Failed to read file: %s\n", e.what());
        return 1;
    }
    print_exif(argv[1]);
    return 0;
}
