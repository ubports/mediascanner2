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
#include <daemon/SubtreeWatcher.hh>

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

const char CRASH_AFTER_ENV[] = "MEDIASCANNER_EXTRACTOR_CRASH_AFTER";

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

void iterate_main_loop() {
    while (g_main_context_iteration(nullptr, FALSE)) {
    }
}

}

class SubtreeWatcherTest : public ::testing::Test {
protected:
    virtual void SetUp() override {
        main_loop_.reset(g_main_loop_new(nullptr, false));

        tmpdir_ = TEST_DIR "/subtreewatcher-test.XXXXXX";
        ASSERT_NE(nullptr, mkdtemp(&tmpdir_[0]));
    }

    void setup_watcher() {
        test_dbus_.reset(g_test_dbus_new(G_TEST_DBUS_NONE));
        g_test_dbus_add_service_dir(test_dbus_.get(), TEST_DIR "/services");
        g_test_dbus_up(test_dbus_.get());
        session_bus_ = make_connection();

        g_dbus_connection_signal_subscribe(
            session_bus_.get(), nullptr,
            "com.canonical.unity.scopes", "InvalidateResults",
            "/com/canonical/unity/scopes",
            "mediascanner-music", G_DBUS_SIGNAL_FLAGS_NONE,
            &SubtreeWatcherTest::invalidateCallback, this, nullptr);

        store_.reset(new MediaStore(":memory:", MS_READ_WRITE));
        extractor_.reset(new MetadataExtractor(session_bus_.get()));
        invalidator_.reset(new InvalidationSender);
        invalidator_->setBus(session_bus_.get());
        watcher_.reset(new SubtreeWatcher(*store_, *extractor_, *invalidator_));
    }

    virtual void TearDown() override {
        watcher_.reset();
        invalidator_.reset();
        extractor_.reset();
        store_.reset();

        session_bus_.reset();
        test_dbus_.reset();

        if (!tmpdir_.empty()) {
            string cmd = "rm -rf " + tmpdir_;
            ASSERT_EQ(0, system(cmd.c_str()));
        }
        unsetenv(CRASH_AFTER_ENV);
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

    bool wait_for_invalidate(int timeout) {
        invalidate_timeout_id_ = g_timeout_add(
            timeout * 1000, [](void *user_data) -> int {
                auto test = reinterpret_cast<SubtreeWatcherTest*>(user_data);
                g_main_loop_quit(test->main_loop_.get());
                test->invalidate_timeout_id_ = 0;
                return G_SOURCE_REMOVE;
            }, this);

        g_main_loop_run(main_loop_.get());

        // The timeout ID will be zero if it was fired and we received
        // no signals.
        if (invalidate_timeout_id_ == 0) {
            return false;
        }

        g_source_remove(invalidate_timeout_id_);
        invalidate_timeout_id_ = 0;
        return true;
    }

    static void invalidateCallback(GDBusConnection */*connection*/,
                                   const char */*sender_name*/,
                                   const char */*object_path*/,
                                   const char */*interface_name*/,
                                   const char */*signal_name*/,
                                   GVariant */*parameters*/,
                                   gpointer user_data) {
        auto test = reinterpret_cast<SubtreeWatcherTest*>(user_data);
        test->invalidate_count_++;

        // If we're waiting on an invalidate signal, quit the main loop
        if (test->invalidate_timeout_id_ != 0) {
            g_main_loop_quit(test->main_loop_.get());
        }
    }

    string tmpdir_;
    unique_ptr<GTestDBus,decltype(&g_object_unref)> test_dbus_ {nullptr, g_object_unref};
    unique_ptr<MediaStore> store_;
    unique_ptr<SubtreeWatcher> watcher_;

    int invalidate_count_ = 0;

private:
    GDBusConnectionPtr session_bus_ {nullptr, g_object_unref};
    unique_ptr<MetadataExtractor> extractor_;
    unique_ptr<InvalidationSender> invalidator_;

    unique_ptr<GMainLoop, decltype(&g_main_loop_unref)> main_loop_ {nullptr, g_main_loop_unref};
    unsigned int invalidate_timeout_id_ = 0;
};

TEST_F(SubtreeWatcherTest, open_for_write_without_change)
{
    setup_watcher();
    watcher_->addDir(tmpdir_);
    iterate_main_loop();

    string testfile = tmpdir_ + "/testfile.ogg";
    copy_file(SOURCE_DIR "/media/testfile.ogg", testfile);
    EXPECT_TRUE(wait_for_invalidate(2));
    ASSERT_EQ(store_->size(), 1);
    // Invalidate called once for new file.
    EXPECT_EQ(1, invalidate_count_);

    int fd = open(testfile.c_str(), O_RDWR);
    ASSERT_GE(fd, 0);
    ASSERT_EQ(0, close(fd));
    // No change, to file so no new invalidations
    EXPECT_FALSE(wait_for_invalidate(2));
    EXPECT_EQ(1, invalidate_count_);

    fd = open(testfile.c_str(), O_RDWR|O_APPEND);
    ASSERT_GE(fd, 0);
    ASSERT_EQ(5, write(fd, "hello", 5));
    ASSERT_EQ(0, close(fd));
    // File changed, so invalidation count increases.
    EXPECT_TRUE(wait_for_invalidate(3));
    EXPECT_EQ(2, invalidate_count_);
}

TEST_F(SubtreeWatcherTest, fallback_added_for_failed_extraction) {
    setenv(CRASH_AFTER_ENV, "0", true);
    setup_watcher();
    watcher_->addDir(tmpdir_);
    iterate_main_loop();

    string testfile = tmpdir_ + "/testfile.ogg";
    copy_file(SOURCE_DIR "/media/testfile.ogg", testfile);
    EXPECT_TRUE(wait_for_invalidate(10));
    ASSERT_EQ(store_->size(), 1);

    // Failed extraction exists in database with its title set based
    // on the file name, but other metadata absent.
    MediaFile media = store_->lookup(testfile);
    EXPECT_EQ("testfile", media.getTitle());
    EXPECT_EQ("", media.getAuthor());
    EXPECT_EQ("", media.getAlbum());
    EXPECT_EQ(0, media.getDuration());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
