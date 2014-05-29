import QtQuick 2.0
import QtTest 1.0
import Ubuntu.MediaScanner 0.1

Item {
    id: root

    MediaStore {
        id: store
    }

    ArtistsModel {
        id: model
        store: store
    }

    TestCase {
        name: "ArtistsModelTests"

        function cleanup() {
            model.albumArtists = false;
            model.genre = undefined;
            model.limit = -1;
        }

        function test_initial_state() {
            compare(model.albumArtists, false);
            compare(model.limit, -1);

            compare(model.rowCount, 2);
            compare(model.get(0, ArtistsModel.RoleArtist), "Spiderbait");
            compare(model.get(1, ArtistsModel.RoleArtist), "The John Butler Trio");
        }

        function test_limit() {
            model.limit = 1;
            compare(model.rowCount, 1);

            model.limit = 42;
            compare(model.rowCount, 2);

            model.limit = -1;
            compare(model.rowCount, 2);
        }

        function test_album_artists() {
            model.albumArtists = true;
            compare(model.rowCount, 2);

            compare(model.get(0, ArtistsModel.RoleArtist), "Spiderbait");
            compare(model.get(1, ArtistsModel.RoleArtist), "The John Butler Trio");
        }

        function test_genre() {
            model.genre = "rock";
            compare(model.rowCount, 1);
            compare(model.get(0, ArtistsModel.RoleArtist), "Spiderbait");

            model.genre = "roots";
            compare(model.rowCount, 1);
            compare(model.get(0, ArtistsModel.RoleArtist), "The John Butler Trio");

            model.genre = "unknown";
            compare(model.rowCount, 0);
        }

    }
}
