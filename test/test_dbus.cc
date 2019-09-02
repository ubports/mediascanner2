#include <gtest/gtest.h>
#include <core/dbus/message.h>
#include <core/dbus/object.h>
#include <core/dbus/types/object_path.h>

#include <mediascanner/Album.hh>
#include <mediascanner/MediaFile.hh>
#include <mediascanner/MediaFileBuilder.hh>
#include <mediascanner/Filter.hh>
#include <ms-dbus/dbus-codec.hh>

class MediaStoreDBusTests : public ::testing::Test {
protected:
    virtual void SetUp() override {
        ::testing::Test::SetUp();
        message = core::dbus::Message::make_method_call(
            "org.example.Name",
            core::dbus::types::ObjectPath("/org/example/Path"),
            "org.example.Interface",
            "Method");
    }

    core::dbus::Message::Ptr message;
};

TEST_F(MediaStoreDBusTests, mediafile_codec) {
    mediascanner::MediaFile media = mediascanner::MediaFileBuilder("a")
        .setContentType("type")
        .setETag("etag")
        .setDate("1900")
        .setTitle("b")
        .setAuthor("c")
        .setAlbum("d")
        .setAlbumArtist("e")
        .setGenre("f")
        .setDiscNumber(0)
        .setTrackNumber(1)
        .setDuration(5)
        .setWidth(640)
        .setHeight(480)
        .setLatitude(20.42)
        .setLongitude(-30.67)
        .setModificationTime(4200)
        .setType(mediascanner::AudioMedia);
    message->writer() << media;

    EXPECT_EQ("(sssssssssiiiiiddbti)", message->signature());
    EXPECT_EQ(core::dbus::helper::TypeMapper<mediascanner::MediaFile>::signature(), message->signature());

    mediascanner::MediaFile media2;
    message->reader() >> media2;
    EXPECT_EQ(media, media2);
}

TEST_F(MediaStoreDBusTests, album_codec) {
    mediascanner::Album album("title", "artist", "date", "genre", "art_file", true, 1);
    message->writer() << album;

    EXPECT_EQ("(sssssb)", message->signature());
    EXPECT_EQ(core::dbus::helper::TypeMapper<mediascanner::Album>::signature(), message->signature());

    mediascanner::Album album2;
    message->reader() >> album2;
    EXPECT_EQ("title", album2.getTitle());
    EXPECT_EQ("artist", album2.getArtist());
    EXPECT_EQ("date", album2.getDate());
    EXPECT_EQ("genre", album2.getGenre());
    EXPECT_EQ("art_file", album2.getArtFile());
    EXPECT_EQ(true, album2.getHasThumbnail());
    EXPECT_EQ(album, album2);
}

TEST_F(MediaStoreDBusTests, filter_codec) {
    mediascanner::Filter filter;
    filter.setArtist("Artist1");
    filter.setAlbum("Album1");
    filter.setAlbumArtist("AlbumArtist1");
    filter.setGenre("Genre");
    filter.setOffset(42);
    filter.setLimit(100);
    message->writer() << filter;

    EXPECT_EQ("a{sv}", message->signature());
    EXPECT_EQ(core::dbus::helper::TypeMapper<mediascanner::Filter>::signature(), message->signature());

    mediascanner::Filter other;
    message->reader() >> other;
    EXPECT_EQ(filter, other);
}

TEST_F(MediaStoreDBusTests, filter_codec_empty) {
    mediascanner::Filter empty;
    message->writer() << empty;

    EXPECT_EQ("a{sv}", message->signature());

    mediascanner::Filter other;
    message->reader() >> other;
    EXPECT_EQ(empty, other);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
