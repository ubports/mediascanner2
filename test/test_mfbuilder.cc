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

/**
 * This is a helper class to build MediaFiles. Since we want them
 * to be immutable and always valid, the user needs to always list
 * all variables in the constructor. This is cumbersome so this class
 * allows you to gather them one by one and then finally construct
 * a fully valid MediaFile.
 *
 * If you try to assign the same property twice, an exception is thrown.
 * This means that MediaFileBuilders are meant to build only one
 * MediaFile. To build a new one create a new MediaFileBuilder. This
 * ensures that no state leaks from the first MediaFile to the second.
 */

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

    MediaType type(AudioMedia);
    std::string fname("abc");
    std::string title("def");
    std::string date("ghi");
    std::string author("jkl");
    std::string album("mno");
    std::string album_artist("pqr");
    int track_number = 13;
    int duration = 99;

    b.setType(type);
    ASSERT_THROW(b.setType(type), std::invalid_argument);

    b.setFilename(fname);
    ASSERT_THROW(b.setFilename(fname), std::invalid_argument);

    b.setTitle(title);
    ASSERT_THROW(b.setTitle(fname), std::invalid_argument);

    b.setDate(date);
    ASSERT_THROW(b.setDate(date), std::invalid_argument);

    b.setAuthor(author);
    ASSERT_THROW(b.setAuthor(author), std::invalid_argument);

    b.setAlbum(album);
    ASSERT_THROW(b.setAlbum(album), std::invalid_argument);

    b.setAlbumArtist(album_artist);
    ASSERT_THROW(b.setAlbumArtist(album_artist), std::invalid_argument);

    b.setTrackNumber(track_number);
    ASSERT_THROW(b.setTrackNumber(track_number), std::invalid_argument);

    b.setDuration(duration);
    ASSERT_THROW(b.setDuration(duration), std::invalid_argument);

    // Now see if data survives a round trip.
    MediaFile mf = b.build();
    ASSERT_EQ(mf.getType(), type);
    ASSERT_EQ(mf.getFileName(), fname);
    ASSERT_EQ(mf.getTitle(), title);
    ASSERT_EQ(mf.getDate(), date);
    ASSERT_EQ(mf.getAuthor(), author);
    ASSERT_EQ(mf.getAlbum(), album);
    ASSERT_EQ(mf.getAlbumArtist(), album_artist);
    ASSERT_EQ(mf.getTrackNumber(), track_number);
    ASSERT_EQ(mf.getDuration(), duration);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
