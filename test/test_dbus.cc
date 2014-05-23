#include <gtest/gtest.h>
#include <core/dbus/message.h>
#include <core/dbus/object.h>
#include <core/dbus/types/object_path.h>

#include <mediascanner/Album.hh>
#include <mediascanner/MediaFile.hh>
#include <mediascanner/MediaFileBuilder.hh>
#include <ms-dbus/dbus-codec.hh>

class MediaStoreDBusTests : public ::testing::Test {
};

TEST_F(MediaStoreDBusTests, mediafile_codec) {
    auto message = core::dbus::Message::make_method_call(
        "org.example.Name",
        core::dbus::types::ObjectPath("/org/example/Path"),
        "org.example.Interface",
        "Method");

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
        .setType(mediascanner::AudioMedia);
    message->writer() << media;

    EXPECT_EQ("(sssssssssiiii)", message->signature());
    EXPECT_EQ(core::dbus::helper::TypeMapper<mediascanner::MediaFile>::signature(), message->signature());

    mediascanner::MediaFile media2;
    message->reader() >> media2;
    EXPECT_EQ(media, media2);
}

TEST_F(MediaStoreDBusTests, album_codec) {
    auto message = core::dbus::Message::make_method_call(
        "org.example.Name",
        core::dbus::types::ObjectPath("/org/example/Path"),
        "org.example.Interface",
        "Method");

    mediascanner::Album album("title", "artist");
    message->writer() << album;

    EXPECT_EQ("(ss)", message->signature());
    EXPECT_EQ(core::dbus::helper::TypeMapper<mediascanner::Album>::signature(), message->signature());

    mediascanner::Album album2;
    message->reader() >> album2;
    EXPECT_EQ("title", album2.getTitle());
    EXPECT_EQ("artist", album2.getArtist());
    EXPECT_EQ(album, album2);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
