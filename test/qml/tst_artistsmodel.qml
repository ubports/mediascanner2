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

    SignalSpy {
        id: modelFilled
        target: model
        signalName: "filled"
    }

    TestCase {
        name: "ArtistsModelTests"

        function cleanup() {
            if (model.albumArtists != false) {
                model.albumArtists = false;
                modelFilled.wait();
            }
            if (model.genre != undefined) {
                model.genre = undefined;
                modelFilled.wait();
            }
        }

        function test_initial_state() {
            compare(model.albumArtists, false);
            compare(model.genre, undefined);

            //modelFilled.wait();
            compare(model.rowCount, 2);
            compare(model.get(0, ArtistsModel.RoleArtist), "Spiderbait");
            compare(model.get(1, ArtistsModel.RoleArtist), "The John Butler Trio");
        }

        function test_limit() {
            // The limit property is deprecated now, but we need to
            // keep it until music-app stops using it.
            compare(model.limit, -1);
            model.limit = 1;
            compare(model.limit, -1);
        }

        function test_album_artists() {
            model.albumArtists = true;
            modelFilled.wait();
            compare(model.rowCount, 2);

            compare(model.get(0, ArtistsModel.RoleArtist), "Spiderbait");
            compare(model.get(1, ArtistsModel.RoleArtist), "The John Butler Trio");
        }

        function test_genre() {
            model.genre = "rock";
            modelFilled.wait();
            compare(model.rowCount, 1);
            compare(model.get(0, ArtistsModel.RoleArtist), "Spiderbait");

            model.genre = "roots";
            modelFilled.wait();
            compare(model.rowCount, 1);
            compare(model.get(0, ArtistsModel.RoleArtist), "The John Butler Trio");

            model.genre = "unknown";
            modelFilled.wait();
            compare(model.rowCount, 0);
        }

    }
}
