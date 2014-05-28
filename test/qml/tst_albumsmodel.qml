import QtQuick 2.0
import QtTest 1.0
import Ubuntu.MediaScanner 0.1

Item {
    id: root

    MediaStore {
        id: store
    }

    AlbumsModel {
        id: model
        store: store
    }

    TestCase {
        name: "AlbumsModelTests"

        function cleanup() {
            model.artist = undefined;
            model.albumArtist = undefined;
            model.limit = -1;
        }

        function test_initial_state() {
            compare(model.artist, undefined);
            compare(model.albumArtist, undefined);
            compare(model.limit, -1);

            compare(model.rowCount, 4);
            compare(model.get(0, AlbumsModel.RoleTitle), "Ivy and the Big Apples");
            compare(model.get(0, AlbumsModel.RoleArtist), "Spiderbait");

            compare(model.get(1, AlbumsModel.RoleTitle), "Spiderbait");
            compare(model.get(1, AlbumsModel.RoleArtist), "Spiderbait");

            compare(model.get(2, AlbumsModel.RoleTitle), "April Uprising");
            compare(model.get(2, AlbumsModel.RoleArtist), "The John Butler Trio");

            compare(model.get(3, AlbumsModel.RoleTitle), "Sunrise Over Sea");
            compare(model.get(3, AlbumsModel.RoleArtist), "The John Butler Trio");
        }

        function test_limit() {
            model.limit = 2;
            compare(model.rowCount, 2);

            model.limit = 42;
            compare(model.rowCount, 4);

            model.limit = -1;
            compare(model.rowCount, 4);
        }

        function test_artist() {
            model.artist = "The John Butler Trio";
            compare(model.rowCount, 2);

            compare(model.get(0, AlbumsModel.RoleTitle), "April Uprising");
            compare(model.get(0, AlbumsModel.RoleArtist), "The John Butler Trio");

            model.artist = "unknown";
            compare(model.rowCount, 0);
        }

        function test_album_artist() {
            model.albumArtist = "The John Butler Trio";
            compare(model.rowCount, 2);

            compare(model.get(0, AlbumsModel.RoleTitle), "April Uprising");
            compare(model.get(0, AlbumsModel.RoleArtist), "The John Butler Trio");

            model.albumArtist = "unknown";
            compare(model.rowCount, 0);
        }

    }
}
