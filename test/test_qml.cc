#include <stdlib.h>
#include <string>

#include <QtQuickTest/quicktest.h>

#include <mediascanner/MediaStore.hh>
#include <mediascanner/MediaFile.hh>
#include <mediascanner/MediaFileBuilder.hh>

using namespace mediascanner;

class MediaStoreData {
public:
    MediaStoreData() {
        db_path = "./mediascanner-cache.XXXXXX";
        if (mkdtemp(const_cast<char*>(db_path.c_str())) == nullptr) {
            throw std::runtime_error("Could not create temporary directory");
        }
        setenv("MEDIASCANNER_CACHEDIR", db_path.c_str(), true);
        populate();
    }
    ~MediaStoreData() {
        if (system("rm -rf \"$MEDIASCANNER_CACHEDIR\"") == -1) {
            throw std::runtime_error("rm -rf failed");
        }
    }

    void populate() {
        MediaStore store(MS_READ_WRITE);

        store.insert(MediaFileBuilder("/path/foo1.ogg")
                     .setType(AudioMedia)
                     .setContentType("audio/ogg")
                     .setETag("etag")
                     .setTitle("Straight Through The Sun")
                     .setAuthor("Spiderbait")
                     .setAlbum("Spiderbait")
                     .setAlbumArtist("Spiderbait")
                     .setDate("2013-11-15")
                     .setGenre("rock")
                     .setDiscNumber(1)
                     .setTrackNumber(1)
                     .setDuration(235));
        store.insert(MediaFileBuilder("/path/foo2.ogg")
                     .setType(AudioMedia)
                     .setContentType("audio/ogg")
                     .setETag("etag")
                     .setTitle("It's Beautiful")
                     .setAuthor("Spiderbait")
                     .setAlbum("Spiderbait")
                     .setAlbumArtist("Spiderbait")
                     .setDate("2013-11-15")
                     .setGenre("rock")
                     .setDiscNumber(1)
                     .setTrackNumber(2)
                     .setDuration(220));

        store.insert(MediaFileBuilder("/path/foo3.ogg")
                     .setType(AudioMedia)
                     .setContentType("audio/ogg")
                     .setETag("etag")
                     .setTitle("Buy Me a Pony")
                     .setAuthor("Spiderbait")
                     .setAlbum("Ivy and the Big Apples")
                     .setAlbumArtist("Spiderbait")
                     .setDate("1996-10-04")
                     .setGenre("rock")
                     .setDiscNumber(1)
                     .setTrackNumber(3)
                     .setDuration(104));

        store.insert(MediaFileBuilder("/path/foo4.ogg")
                     .setType(AudioMedia)
                     .setContentType("audio/ogg")
                     .setETag("etag")
                     .setTitle("Peaches & Cream")
                     .setAuthor("The John Butler Trio")
                     .setAlbum("Sunrise Over Sea")
                     .setAlbumArtist("The John Butler Trio")
                     .setDate("2004-03-08")
                     .setGenre("roots")
                     .setDiscNumber(1)
                     .setTrackNumber(2)
                     .setDuration(407));
        store.insert(MediaFileBuilder("/path/foo5.ogg")
                     .setType(AudioMedia)
                     .setContentType("audio/ogg")
                     .setETag("etag")
                     .setTitle("Zebra")
                     .setAuthor("The John Butler Trio")
                     .setAlbum("Sunrise Over Sea")
                     .setAlbumArtist("The John Butler Trio")
                     .setDate("2004-03-08")
                     .setGenre("roots")
                     .setDiscNumber(1)
                     .setTrackNumber(10)
                     .setDuration(237));

        store.insert(MediaFileBuilder("/path/foo6.ogg")
                     .setType(AudioMedia)
                     .setContentType("audio/ogg")
                     .setETag("etag")
                     .setTitle("Revolution")
                     .setAuthor("The John Butler Trio")
                     .setAlbum("April Uprising")
                     .setAlbumArtist("The John Butler Trio")
                     .setDate("2010-01-01")
                     .setGenre("roots")
                     .setDiscNumber(1)
                     .setTrackNumber(1)
                     .setDuration(305));
        store.insert(MediaFileBuilder("/path/foo7.ogg")
                     .setType(AudioMedia)
                     .setContentType("audio/ogg")
                     .setETag("etag")
                     .setTitle("One Way Road")
                     .setAuthor("The John Butler Trio")
                     .setAlbum("April Uprising")
                     .setAlbumArtist("The John Butler Trio")
                     .setDate("2010-01-01")
                     .setGenre("roots")
                     .setDiscNumber(1)
                     .setTrackNumber(2)
                     .setDuration(185));
    }

private:
    std::string db_path;
};

MediaStoreData data;


QUICK_TEST_MAIN(Mediascaner)
