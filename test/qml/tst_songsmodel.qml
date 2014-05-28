import QtQuick 2.0
import QtTest 1.0
import Ubuntu.MediaScanner 0.1

Item {
    id: root

    MediaStore {
        id: store
    }

    SongsModel {
        id: model
        store: store
    }

    TestCase {
        name: "SongsModelTests"

        function cleanup() {
            model.artist = undefined;
            model.albumArtist = undefined;
            model.album = undefined;
            model.limit = -1;
        }

        function test_initial_state() {
            compare(model.artist, undefined);
            compare(model.albumArtist, undefined);
            compare(model.album, undefined);
            compare(model.limit, -1);

            compare(model.rowCount, 7);
            compare(model.get(0, SongsModel.RoleTitle), "Buy Me a Pony")
            compare(model.get(0, SongsModel.RoleAlbum), "Ivy and the Big Apples");
            compare(model.get(0, SongsModel.RoleAuthor), "Spiderbait");

            compare(model.get(1, SongsModel.RoleTitle), "Straight Through The Sun");
            compare(model.get(2, SongsModel.RoleTitle), "It's Beautiful");
            compare(model.get(3, SongsModel.RoleTitle), "Revolution");
            compare(model.get(4, SongsModel.RoleTitle), "One Way Road");
            compare(model.get(5, SongsModel.RoleTitle), "Peaches & Cream");
            compare(model.get(6, SongsModel.RoleTitle), "Zebra");
        }

        function test_limit() {
            model.limit = 2;
            compare(model.rowCount, 2);

            model.limit = 42;
            compare(model.rowCount, 7);

            model.limit = -1;
            compare(model.rowCount, 7);
        }

        function test_artist() {
            model.artist = "The John Butler Trio";
            compare(model.rowCount, 4);

            compare(model.get(0, SongsModel.RoleTitle), "Revolution");
            compare(model.get(0, SongsModel.RoleAuthor), "The John Butler Trio");

            model.artist = "unknown";
            compare(model.rowCount, 0);
        }

        function test_album_artist() {
            model.albumArtist = "The John Butler Trio";
            compare(model.rowCount, 4);

            compare(model.get(0, SongsModel.RoleTitle), "Revolution");
            compare(model.get(0, SongsModel.RoleAuthor), "The John Butler Trio");

            model.albumArtist = "unknown";
            compare(model.rowCount, 0);
        }

        function test_album() {
            model.album = "Sunrise Over Sea";
            compare(model.rowCount, 2);

            compare(model.get(0, SongsModel.RoleTitle), "Peaches & Cream");
            compare(model.get(0, SongsModel.RoleAuthor), "The John Butler Trio");

            model.albumArtist = "unknown";
            compare(model.rowCount, 0);
        }

    }
}
