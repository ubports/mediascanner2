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

#pragma once

#include <memory>
#include <string>

namespace mediascanner {

class MediaStore;
class MetadataExtractor;
class InvalidationSender;

struct VolumeManagerPrivate;

class VolumeManager final {
public:
    VolumeManager(MediaStore& store, MetadataExtractor& extractor, InvalidationSender& invalidator);
    ~VolumeManager();
    VolumeManager(const VolumeManager&) = delete;
    VolumeManager& operator=(const VolumeManager&) = delete;

    void queueAddVolume(const std::string& path);
    void queueRemoveVolume(const std::string& path);

    bool idle() const;

private:
    std::unique_ptr<VolumeManagerPrivate> p;
};

}
