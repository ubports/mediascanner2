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
        id: modelStatus
        target: model
        signalName: "statusChanged"
    }

    TestCase {
        name: "ArtistsModelTests"

        function waitForReady() {
            while (model.status == ArtistsModel.Loading) {
                modelStatus.wait();
            }
            compare(model.status, ArtistsModel.Ready);
        }

        function cleanup() {
            model.albumArtists = false;
            model.genre = undefined;
            waitForReady();
        }

        function test_initial_state() {
            compare(model.albumArtists, false);
            compare(model.genre, undefined);

            waitForReady();
            compare(model.count, 2);
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
            waitForReady();
            compare(model.count, 2);

            compare(model.get(0, ArtistsModel.RoleArtist), "Spiderbait");
            compare(model.get(1, ArtistsModel.RoleArtist), "The John Butler Trio");
        }

        function test_genre() {
            model.genre = "rock";
            waitForReady();
            compare(model.count, 1);
            compare(model.get(0, ArtistsModel.RoleArtist), "Spiderbait");

            model.genre = "roots";
            waitForReady();
            compare(model.count, 1);
            compare(model.get(0, ArtistsModel.RoleArtist), "The John Butler Trio");

            model.genre = "unknown";
            waitForReady();
            compare(model.count, 0);
        }

    }
}
