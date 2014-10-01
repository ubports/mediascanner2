/*
 * Copyright (C) 2014 Canonical, Ltd.
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

#include "MountWatcher.hh"
#include <cstdio>
#include <cstring>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>

#include <glib.h>
#include <gio/gio.h>
#include <udisks/udisks.h>

namespace {
const char BLOCK_DEVICE_PREFIX[] = "/org/freedesktop/UDisks2/block_devices/";
const char FILESYSTEM_IFACE[] = "org.freedesktop.UDisks2.Filesystem";

struct DeviceInfo {
    mediascanner::MountWatcherPrivate *p;
    std::unique_ptr<UDisksObject, void(*)(void*)> device;
    std::unique_ptr<UDisksFilesystem, void(*)(void*)> filesystem;
    std::string mount_point;
    bool is_mounted = false;
    unsigned int mount_point_changed_id = 0;

    DeviceInfo(mediascanner::MountWatcherPrivate *p, UDisksObject *dev);
    ~DeviceInfo();

    DeviceInfo(const DeviceInfo&) = delete;
    DeviceInfo& operator=(DeviceInfo& o) = delete;

    void filesystem_added();
    void filesystem_removed();
    void update_mount_state();
    static void mount_point_changed(GObject *, GParamSpec *,
                                    void *user_data) noexcept;
};
}

namespace mediascanner {


struct MountWatcherPrivate {
    std::function<void(const MountWatcher::Info&)> callback;
    std::unique_ptr<UDisksClient, void(*)(void*)> client;
    GDBusObjectManager *manager = nullptr;
    std::map<std::string,std::unique_ptr<DeviceInfo>> devices;

    unsigned int object_added_id = 0;
    unsigned int object_removed_id = 0;
    unsigned int interface_added_id = 0;
    unsigned int interface_removed_id = 0;

    MountWatcherPrivate(std::function<void(const MountWatcher::Info&)> callback);
    ~MountWatcherPrivate();

    static void object_added(GDBusObjectManager *manager,
                             GDBusObject *object, void *user_data) noexcept;
    static void object_removed(GDBusObjectManager *manager,
                               GDBusObject *object, void *user_data) noexcept;
    static void interface_added(GDBusObjectManager *manager,
                                GDBusObject *object, GDBusInterface *iface,
                                void *user_data) noexcept;
    static void interface_removed(GDBusObjectManager *manager,
                                  GDBusObject *object, GDBusInterface *iface,
                                  void *user_data) noexcept;
};

MountWatcher::MountWatcher(std::function<void(const Info&)> callback)
    : p(new MountWatcherPrivate(callback)) {
    GError *error = nullptr;
    p->client.reset(udisks_client_new_sync(nullptr, &error));
    if (not p->client) {
        std::string errortxt(error->message);
        g_error_free(error);
        delete(p);

        throw std::runtime_error(
            std::string("Failed to create udisks2 client: ") + errortxt);
    }
    p->manager = udisks_client_get_object_manager(p->client.get());

    p->object_added_id = g_signal_connect(
        p->manager, "object-added",
        G_CALLBACK(&MountWatcherPrivate::object_added), p);
    p->object_removed_id = g_signal_connect(
        p->manager, "object-removed",
        G_CALLBACK(&MountWatcherPrivate::object_removed), p);
    p->interface_added_id = g_signal_connect(
        p->manager, "interface-added",
        G_CALLBACK(&MountWatcherPrivate::interface_added), p);
    p->interface_removed_id = g_signal_connect(
        p->manager, "interface-removed",
        G_CALLBACK(&MountWatcherPrivate::interface_removed), p);

    GList *objects = g_dbus_object_manager_get_objects(p->manager);
    for (GList *l = objects; l != nullptr; l = l->next) {
        GDBusObject *object = reinterpret_cast<GDBusObject*>(l->data);
        MountWatcherPrivate::object_added(p->manager, object, p);
    }
}

MountWatcher::~MountWatcher() {
    delete p;
}

MountWatcherPrivate::MountWatcherPrivate(
    std::function<void(const MountWatcher::Info&)> callback)
    : callback(callback), client(nullptr, g_object_unref) {
}

MountWatcherPrivate::~MountWatcherPrivate() {
    if (object_added_id != 0) {
        g_signal_handler_disconnect(manager, object_added_id);
    }
    if (object_removed_id != 0) {
        g_signal_handler_disconnect(manager, object_removed_id);
    }
    if (interface_added_id != 0) {
        g_signal_handler_disconnect(manager, interface_added_id);
    }
    if (interface_removed_id != 0) {
        g_signal_handler_disconnect(manager, interface_removed_id);
    }
}

void MountWatcherPrivate::object_added(GDBusObjectManager *, GDBusObject *object, void *user_data) noexcept {
    MountWatcherPrivate *p = reinterpret_cast<MountWatcherPrivate*>(user_data);

    // We're only interested in block devices
    const char *object_path = g_dbus_object_get_object_path(object);
    if (!g_str_has_prefix(object_path, BLOCK_DEVICE_PREFIX)) {
        return;
    }
    UDisksObject *device = UDISKS_OBJECT(object);
    UDisksBlock *block = udisks_object_peek_block(device);
    if (not block) {
        return;
    }

    // Check if we're already tracking this object (this should never
    // be true if events are received in order, but it doesn't hurt to
    // check).
    if (p->devices.find(object_path) != p->devices.end()) {
        return;
    }

    // Determine whether the device belongs to a removable drive.
    std::unique_ptr<UDisksObject, void(*)(void*)> drive_object(
        udisks_client_get_object(p->client.get(),
                                 udisks_block_get_drive(block)),
        g_object_unref);
    if (not drive_object) {
        return;
    }
    UDisksDrive *drive = udisks_object_peek_drive(drive_object.get());
    if (not drive || !udisks_drive_get_removable(drive))
        return;

    // Start tracking this device
    std::unique_ptr<DeviceInfo> info(new DeviceInfo(p, device));
    p->devices.emplace(object_path, std::move(info));
}

void MountWatcherPrivate::object_removed(GDBusObjectManager *, GDBusObject *object, void *user_data) noexcept {
    MountWatcherPrivate *p = reinterpret_cast<MountWatcherPrivate*>(user_data);
    const char *object_path = g_dbus_object_get_object_path(object);
    p->devices.erase(object_path);
}

void MountWatcherPrivate::interface_added(GDBusObjectManager *, GDBusObject *object, GDBusInterface *iface, void *user_data) noexcept {
    // We're only interested in the filesystem interface
    GDBusInterfaceInfo *iface_info = g_dbus_interface_get_info(iface);
    if (strcmp(iface_info->name, FILESYSTEM_IFACE) != 0) {
        return;
    }

    MountWatcherPrivate *p = reinterpret_cast<MountWatcherPrivate*>(user_data);
    const char *object_path = g_dbus_object_get_object_path(object);
    try {
        p->devices.at(object_path)->filesystem_added();
    } catch (const std::out_of_range &e) {
    }
}

void MountWatcherPrivate::interface_removed(GDBusObjectManager *, GDBusObject *object, GDBusInterface *iface, void *user_data) noexcept {
    // We're only interested in the filesystem interface
    GDBusInterfaceInfo *iface_info = g_dbus_interface_get_info(iface);
    if (strcmp(iface_info->name, FILESYSTEM_IFACE) != 0) {
        return;
    }

    MountWatcherPrivate *p = reinterpret_cast<MountWatcherPrivate*>(user_data);
    const char *object_path = g_dbus_object_get_object_path(object);
    try {
        p->devices.at(object_path)->filesystem_removed();
    } catch (const std::out_of_range &e) {
    }
}

}

namespace {

DeviceInfo::DeviceInfo(mediascanner::MountWatcherPrivate *p, UDisksObject *dev)
    : p(p),
      device(reinterpret_cast<UDisksObject*>(g_object_ref(dev)),
             g_object_unref),
      filesystem(nullptr, g_object_unref) {
    if (udisks_object_peek_filesystem(device.get())) {
        filesystem_added();
    }
}

DeviceInfo::~DeviceInfo() {
    filesystem_removed();
}

void DeviceInfo::filesystem_added() {
    if (mount_point_changed_id != 0) {
        g_signal_handler_disconnect(filesystem.get(), mount_point_changed_id);
        mount_point_changed_id = 0;
    }
    filesystem.reset(udisks_object_get_filesystem(device.get()));

    mount_point_changed_id = g_signal_connect(
        filesystem.get(), "notify::mount-points",
        G_CALLBACK(&DeviceInfo::mount_point_changed), this);
    update_mount_state();
}

void DeviceInfo::filesystem_removed() {
    if (mount_point_changed_id != 0) {
        g_signal_handler_disconnect(filesystem.get(), mount_point_changed_id);
        mount_point_changed_id = 0;
    }
    filesystem.reset();
    update_mount_state();
}

void DeviceInfo::update_mount_state() {
    const char *new_mount_point = nullptr;
    if (filesystem) {
        auto mount_points = udisks_filesystem_get_mount_points(filesystem.get());
        new_mount_point = mount_points ? mount_points[0] : nullptr;
    }

    // Has the mount state changed?
    if ((new_mount_point != nullptr) == is_mounted) {
        return;
    }

    // Construct an info structure for the callback
    mediascanner::MountWatcher::Info mount_info;
    mount_info.is_mounted = new_mount_point != nullptr;
    mount_info.mount_point =
        mount_info.is_mounted ? std::string(new_mount_point) : mount_point;
    UDisksBlock *block = udisks_object_peek_block(device.get());
    const char *device = udisks_block_get_device(block);
    if (device != nullptr) {
        mount_info.device = device;
    }
    const char *uuid = udisks_block_get_id_uuid(block);
    if (uuid != nullptr) {
        mount_info.uuid = uuid;
    }

    // And then update our internal state
    is_mounted = new_mount_point != nullptr;
    if (is_mounted) {
        mount_point = new_mount_point;
    } else {
        mount_point = "";
    }

    p->callback(mount_info);
}

void DeviceInfo::mount_point_changed(GObject *, GParamSpec *,
                                     void *user_data) noexcept {
    DeviceInfo *info = reinterpret_cast<DeviceInfo*>(user_data);
    info->update_mount_state();
}

}
