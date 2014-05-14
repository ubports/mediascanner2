#include <stdlib.h>
#include <string>

#include <QtQuickTest/quicktest.h>

#include <mediascanner/MediaStore.hh>
#include <mediascanner/MediaFile.hh>

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

        store.insert(MediaFile("/path/foo1.ogg", "audio/ogg", "etag",
                               "Straight Through The Sun", "2013-11-15", "Spiderbait",
                               "Spiderbait", "Spiderbait", "rock", 1, 1, 235, AudioMedia));
        store.insert(MediaFile("/path/foo2.ogg", "audio/ogg", "etag",
                               "It's Beautiful", "2013-11-15", "Spiderbait",
                               "Spiderbait", "Spiderbait", "rock", 1, 2, 220, AudioMedia));

        store.insert(MediaFile("/path/foo3.ogg", "audio/ogg", "etag",
                               "Buy Me a Pony", "1996-10-04", "Spiderbait",
                               "Ivy and the Big Apples", "Spiderbait", "rock", 1, 3, 104, AudioMedia));

        store.insert(MediaFile("/path/foo4.ogg", "audio/ogg", "etag",
                               "Peaches & Cream", "2004-03-08", "The John Butler Trio",
                               "Sunrise Over Sea", "The John Butler Trio", "roots", 1, 2, 407, AudioMedia));
        store.insert(MediaFile("/path/foo5.ogg", "audio/ogg", "etag",
                               "Zebra", "2004-03-08", "The John Butler Trio",
                               "Sunrise Over Sea", "The John Butler Trio", "roots", 1, 10, 237, AudioMedia));

        store.insert(MediaFile("/path/foo6.ogg", "audio/ogg", "etag",
                               "Revolution", "2010-01-01", "The John Butler Trio",
                               "April Uprising", "The John Butler Trio", "roots", 1, 1, 305, AudioMedia));
        store.insert(MediaFile("/path/foo7.ogg", "audio/ogg", "etag",
                               "One Way Road", "2010-01-01", "The John Butler Trio",
                               "April Uprising", "The John Butler Trio", "roots", 1, 2, 185, AudioMedia));
    }

private:
    std::string db_path;
};

MediaStoreData data;


QUICK_TEST_MAIN(Mediascaner)
