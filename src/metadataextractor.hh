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

#ifndef METADATAEXTRACTOR_H
#define METADATAEXTRACTOR_H

#include "MediaFile.hh"
#include<string>
#include<memory>

typedef struct _GstDiscoverer GstDiscoverer;

class MetadataExtractor {
public:
    MetadataExtractor(int seconds=25);

    MediaFile extract(const std::string &filename);
private:
    std::unique_ptr<GstDiscoverer,void(*)(void *)> discoverer;
};


int getMetadata(const std::string &filename, std::string &title, std::string &author, std::string &album,
        int &duration);
int getDuration(const std::string &filename);

#endif
