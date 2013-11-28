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

#include<gtest/gtest.h>
#include<stdexcept>
#include"mediascanner/MediaFile.hh"
#include"mediascanner/MediaFileBuilder.hh"

class MFBTest : public ::testing::Test {
 protected:
  MFBTest() {
  }

  virtual ~MFBTest() {
  }

  virtual void SetUp() {
  }

  virtual void TearDown() {
  }
};

TEST_F(MFBTest, basic) {
    MediaFileBuilder b;
    ASSERT_THROW(b.build(), std::invalid_argument);
    MediaType type(AudioMedia);
    b.setType(type);
    ASSERT_THROW(b.setType(type), std::invalid_argument);
    ASSERT_THROW(b.build(), std::invalid_argument);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
