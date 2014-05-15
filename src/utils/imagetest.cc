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

#include<libexif/exif-loader.h>
#include<iostream>

static void print_exif(const char *infile) {
    ExifLoader *l;
    l = exif_loader_new();
    ExifData *ed;
    exif_loader_write_file(l, infile);
    ed = exif_loader_get_data(l);
    exif_loader_unref(l);
    exif_data_unref(ed);
}

int main(int argc, char **argv) {
    if(argc != 2) {
        std::cout << argv[0] << "image_file" << std::endl;
        return 1;
    }
    print_exif(argv[1]);
    return 0;
}
