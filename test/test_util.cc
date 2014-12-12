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

#include <gtest/gtest.h>
#include"../src/mediascanner/internal/utils.hh"
#include"../src/mediascanner/MediaFile.hh"
#include"../src/mediascanner/MediaFileBuilder.hh"

#include "test_config.h"

using namespace mediascanner;

class UtilTest : public ::testing::Test {
public:
    UtilTest() {
    }

    virtual void SetUp() {
    }

    virtual void TearDown() {
    }

};
TEST_F(UtilTest, optical) {
    std::string blu_root(SOURCE_DIR "/bluray_root");
    std::string dvd_root(SOURCE_DIR "/dvd_root");
    std::string nodisc_root(SOURCE_DIR "/media");
    ASSERT_TRUE(is_optical_disc(blu_root));
    ASSERT_TRUE(is_optical_disc(dvd_root));
    ASSERT_FALSE(is_optical_disc(nodisc_root));
}

TEST_F(UtilTest, scanblock) {
    std::string noblock_root(SOURCE_DIR "/media");
    std::string block_root(SOURCE_DIR "/noscan");
    ASSERT_TRUE(has_scanblock(block_root));
    ASSERT_FALSE(has_scanblock(noblock_root));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
