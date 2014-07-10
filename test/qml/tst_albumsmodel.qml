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

    SignalSpy {
        id: modelFilled
        target: model
        signalName: "filled"
    }

    TestCase {
        name: "AlbumsModelTests"

        function cleanup() {
            if (model.artist != undefined) {
                model.artist = undefined;
                modelFilled.wait();
            }
            if (model.albumArtist != undefined) {
                model.albumArtist = undefined;
                modelFilled.wait();
            }
            if (model.genre != undefined) {
                model.genre = undefined;
                modelFilled.wait();
            }
        }

        function test_initial_state() {
            compare(model.artist, undefined);
            compare(model.albumArtist, undefined);
            compare(model.genre, undefined);
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
            // The limit property is deprecated now, but we need to
            // keep it until music-app stops using it.
            compare(model.limit, -1);
            model.limit = 1;
            compare(model.limit, -1);
        }

        function test_artist() {
            model.artist = "The John Butler Trio";
            modelFilled.wait();
            compare(model.rowCount, 2);

            compare(model.get(0, AlbumsModel.RoleTitle), "April Uprising");
            compare(model.get(0, AlbumsModel.RoleArtist), "The John Butler Trio");

            model.artist = "unknown";
            modelFilled.wait();
            compare(model.rowCount, 0);
        }

        function test_album_artist() {
            model.albumArtist = "The John Butler Trio";
            modelFilled.wait();
            compare(model.rowCount, 2);

            compare(model.get(0, AlbumsModel.RoleTitle), "April Uprising");
            compare(model.get(0, AlbumsModel.RoleArtist), "The John Butler Trio");

            model.albumArtist = "unknown";
            modelFilled.wait();
            compare(model.rowCount, 0);
        }

        function test_genre() {
            model.genre = "rock";
            modelFilled.wait();
            compare(model.rowCount, 2);
            compare(model.get(0, AlbumsModel.RoleTitle), "Ivy and the Big Apples");
            compare(model.get(1, AlbumsModel.RoleTitle), "Spiderbait");

            model.genre = "unknown";
            modelFilled.wait();
            compare(model.rowCount, 0);
        }

    }
}
