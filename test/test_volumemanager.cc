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

#include <mediascanner/MediaFile.hh>
#include <mediascanner/MediaStore.hh>
#include <extractor/MetadataExtractor.hh>
#include <daemon/InvalidationSender.hh>
#include <daemon/VolumeManager.hh>

#include "test_config.h"

#include <fcntl.h>
#include <algorithm>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>

#include <gio/gio.h>
#include <gtest/gtest.h>

using namespace std;
using namespace mediascanner;

namespace {

typedef std::unique_ptr<GDBusConnection, decltype(&g_object_unref)> GDBusConnectionPtr;

void copy_file(const string &src, const string &dst) {
    FILE* f = fopen(src.c_str(), "r");
    ASSERT_TRUE(f);
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);

    char* buf = new char[size];

    fseek(f, 0, SEEK_SET);
    ASSERT_EQ(fread(buf, 1, size, f), size);
    fclose(f);

    f = fopen(dst.c_str(), "w");
    ASSERT_TRUE(f);
    ASSERT_EQ(fwrite(buf, 1, size, f), size);
    fclose(f);
    delete[] buf;
}

}

class VolumeManagerTest : public ::testing::Test {
protected:
    virtual void SetUp() override {
        main_loop_.reset(g_main_loop_new(nullptr, false));

        tmpdir_ = TEST_DIR "/volumemanager-test.XXXXXX";
        ASSERT_NE(nullptr, mkdtemp(&tmpdir_[0]));

        test_dbus_.reset(g_test_dbus_new(G_TEST_DBUS_NONE));
        g_test_dbus_add_service_dir(test_dbus_.get(), TEST_DIR "/services");
        g_test_dbus_up(test_dbus_.get());
        session_bus_ = make_connection();

        store_.reset(new MediaStore(":memory:", MS_READ_WRITE));
        extractor_.reset(new MetadataExtractor(session_bus_.get()));
        invalidator_.reset(new InvalidationSender);
        volumes_.reset(new VolumeManager(*store_, *extractor_, *invalidator_));
    }

    virtual void TearDown() override {
        volumes_.reset();
        invalidator_.reset();
        extractor_.reset();
        store_.reset();

        session_bus_.reset();
        test_dbus_.reset();

        if (!tmpdir_.empty()) {
            string cmd = "rm -rf " + tmpdir_;
            ASSERT_EQ(0, system(cmd.c_str()));
        }
    }

    GDBusConnectionPtr make_connection() {
        GError *error = nullptr;
        char *address = g_dbus_address_get_for_bus_sync(G_BUS_TYPE_SESSION, nullptr, &error);
        if (!address) {
            string errortxt(error->message);
            g_error_free(error);
            throw std::runtime_error(
                string("Failed to determine session bus address: ") + errortxt);
        }
        GDBusConnectionPtr bus(
            g_dbus_connection_new_for_address_sync(
                address, static_cast<GDBusConnectionFlags>(G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT | G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION), nullptr, nullptr, &error),
            g_object_unref);
        g_free(address);
        if (!bus) {
            string errortxt(error->message);
            g_error_free(error);
            throw std::runtime_error(
                string("Failed to connect to session bus: ") + errortxt);
        }
        return std::move(bus);
    }

    void wait_until_idle() {
        while (g_main_context_iteration(nullptr, FALSE)) {
            if (volumes_->idle()) {
                break;
            }
        }
    }

    string tmpdir_;
    unique_ptr<GTestDBus,decltype(&g_object_unref)> test_dbus_ {nullptr, g_object_unref};
    unique_ptr<MediaStore> store_;
    unique_ptr<VolumeManager> volumes_;

private:
    GDBusConnectionPtr session_bus_ {nullptr, g_object_unref};
    unique_ptr<MetadataExtractor> extractor_;
    unique_ptr<InvalidationSender> invalidator_;

    unique_ptr<GMainLoop, decltype(&g_main_loop_unref)> main_loop_ {nullptr, g_main_loop_unref};
};

TEST_F(VolumeManagerTest, add_remove_volumes)
{
    const string volume1 = tmpdir_ + "/volume1";
    const string volume2 = tmpdir_ + "/volume2";
    ASSERT_EQ(0, mkdir(volume1.c_str(), 0755));
    ASSERT_EQ(0, mkdir(volume2.c_str(), 0755));

    const string file1 = volume1 + "/file1.ogg";
    const string file2 = volume1 + "/file2.ogg";
    const string file3 = volume2 + "/file3.ogg";

    copy_file(SOURCE_DIR "/media/testfile.ogg", file1);
    copy_file(SOURCE_DIR "/media/testfile.ogg", file2);
    copy_file(SOURCE_DIR "/media/testfile.ogg", file3);

    volumes_->queueAddVolume(volume1);
    wait_until_idle();
    EXPECT_EQ(store_->size(), 2);
    store_->lookup(file1);
    store_->lookup(file2);
    try {
        store_->lookup(file3);
        FAIL();
    } catch (const runtime_error&) {
    }

    // Queue up two operations
    volumes_->queueAddVolume(volume2);
    volumes_->queueRemoveVolume(volume1);
    wait_until_idle();
    EXPECT_EQ(store_->size(), 1);
    store_->lookup(file3);
    try {
        store_->lookup(file1);
        store_->lookup(file2);
    } catch (const runtime_error&) {
    }

    volumes_->queueRemoveVolume(volume2);
    wait_until_idle();
    EXPECT_EQ(store_->size(), 0);

    // This should result in only volume2 being added
    volumes_->queueAddVolume(volume1);
    volumes_->queueAddVolume(volume2);
    volumes_->queueRemoveVolume(volume1);
    wait_until_idle();
    EXPECT_EQ(store_->size(), 1);
    store_->lookup(file3);
}

TEST_F(VolumeManagerTest, add_volume_during_initial_scan)
{
    const string volume1 = tmpdir_ + "/volume1";
    const string volume2 = tmpdir_ + "/volume2";
    ASSERT_EQ(0, mkdir(volume1.c_str(), 0755));
    ASSERT_EQ(0, mkdir(volume2.c_str(), 0755));

    const string file1 = volume1 + "/file1.ogg";
    const string file2 = volume1 + "/file2.ogg";
    const string file3 = volume2 + "/file3.ogg";

    copy_file(SOURCE_DIR "/media/testfile.ogg", file1);
    copy_file(SOURCE_DIR "/media/testfile.ogg", file2);
    copy_file(SOURCE_DIR "/media/testfile.ogg", file3);

    // Set an idle function that should be called during the initial
    // scan that will queue up adding a second volume.
    function<void()> callback = [&] {
        // We expect that this is called during the scan
        EXPECT_FALSE(volumes_->idle());
        volumes_->queueAddVolume(volume2);
    };
    g_idle_add([](void *user_data) -> gboolean {
            auto callback = *reinterpret_cast<function<void()>*>(user_data);
            callback();
            return G_SOURCE_REMOVE;
        }, &callback);

    volumes_->queueAddVolume(volume1);
    wait_until_idle();
    EXPECT_EQ(store_->size(), 3);
    store_->lookup(file1);
    store_->lookup(file2);
    store_->lookup(file3);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
