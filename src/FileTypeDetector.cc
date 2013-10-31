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

#include"FileTypeDetector.hh"
#include<string.h>
#include<gio/gio.h>
#include<cstdio>
#include<memory>

MediaType FileTypeDetector::detect(const std::string &fname) {
    std::unique_ptr<GFile, void(*)(void *)> file(
        g_file_new_for_path(fname.c_str()), g_object_unref);
    if (!file) {
        return UnknownMedia;
    }

    std::unique_ptr<GFileInfo, void(*)(void *)> info(
        g_file_query_info(
            file.get(), G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE,
            G_FILE_QUERY_INFO_NONE, /* cancellable */ NULL, /* error */NULL),
        g_object_unref);
    if (!info) {
        return UnknownMedia;
    }

    const char *content_type = g_file_info_get_content_type(info.get());
    if (!content_type) {
        return UnknownMedia;
    }

    if (!strncmp(content_type, "audio/", strlen("audio/"))) {
        return AudioMedia;
    }
    if (!strncmp(content_type, "video/", strlen("video/"))) {
        return VideoMedia;
    }
    return UnknownMedia;
}
