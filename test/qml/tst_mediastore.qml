import QtQuick 2.0
import QtTest 1.0
import Ubuntu.MediaScanner 0.1

Item {
    id: root

    MediaStore {
        id: store
    }


    TestCase {
        name: "MediaStoreTests"

        function test_lookup() {
            var song = store.lookup("/unknown.ogg");
            compare(song, null, "song == null");

            song = store.lookup("/path/foo1.ogg");
            verify(song !== null, "song != null");
            var checkAttr = function (attr, value) {
                compare(song[attr], value, "song." + attr + " == \"" + value + "\"");
            };
            checkAttr("filename", "/path/foo1.ogg");
            checkAttr("uri", "file:///path/foo1.ogg");
            checkAttr("contentType", "audio/ogg");
            checkAttr("eTag", "etag");
            checkAttr("title", "Straight Through The Sun");
            checkAttr("author", "Spiderbait");
            checkAttr("album", "Spiderbait");
            checkAttr("albumArtist", "Spiderbait");
            checkAttr("date", "2013-11-15");
            checkAttr("genre", "rock");
            checkAttr("discNumber", 1);
            checkAttr("trackNumber", 1);
            checkAttr("duration", 235);
            checkAttr("width", 0);
            checkAttr("height", 0);
            checkAttr("latitude", 0.0);
            checkAttr("longitude", 0.0);
            checkAttr("art", "image://albumart/artist=Spiderbait&album=Spiderbait");
        }

        function test_query() {
            var songs = store.query("unknown", MediaStore.AudioMedia);
            compare(songs.length, 0, "songs.length == 0");

            var songs = store.query("Pony", MediaStore.AudioMedia);
            compare(songs.length, 1, "songs.length == 1");
            compare(songs[0].title, "Buy Me a Pony");
        }
    }
}
