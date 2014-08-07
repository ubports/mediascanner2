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

    SignalSpy {
        id: modelStatus
        target: model
        signalName: "statusChanged"
    }

    TestCase {
        name: "SongsModelTests"

        function waitForReady() {
            while (model.status == SongsModel.Loading) {
                modelStatus.wait();
            }
            compare(model.status, SongsModel.Ready);
        }

        function cleanup() {
            model.artist = undefined;
            model.albumArtist = undefined;
            model.album = undefined;
            model.genre = undefined;
            waitForReady();
        }

        function test_initial_state() {
            compare(model.artist, undefined);
            compare(model.albumArtist, undefined);
            compare(model.album, undefined);

            compare(model.count, 7);
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
            // The limit property is deprecated now, but we need to
            // keep it until music-app stops using it.
            compare(model.limit, -1);
            model.limit = 1;
            compare(model.limit, -1);
        }

        function test_artist() {
            model.artist = "The John Butler Trio";
            waitForReady();
            compare(model.count, 4);

            compare(model.get(0, SongsModel.RoleTitle), "Revolution");
            compare(model.get(0, SongsModel.RoleAuthor), "The John Butler Trio");

            model.artist = "unknown";
            waitForReady();
            compare(model.count, 0);
        }

        function test_album_artist() {
            model.albumArtist = "The John Butler Trio";
            waitForReady();
            compare(model.count, 4);

            compare(model.get(0, SongsModel.RoleTitle), "Revolution");
            compare(model.get(0, SongsModel.RoleAuthor), "The John Butler Trio");

            model.albumArtist = "unknown";
            waitForReady();
            compare(model.count, 0);
        }

        function test_album() {
            model.album = "Sunrise Over Sea";
            waitForReady();
            compare(model.count, 2);

            compare(model.get(0, SongsModel.RoleTitle), "Peaches & Cream");
            compare(model.get(0, SongsModel.RoleAuthor), "The John Butler Trio");

            model.albumArtist = "unknown";
            waitForReady();
            compare(model.count, 0);
        }

        function test_genre() {
            model.genre = "rock";
            waitForReady();
            compare(model.count, 3);

            compare(model.get(0, SongsModel.RoleTitle), "Buy Me a Pony");
            compare(model.get(1, SongsModel.RoleTitle), "Straight Through The Sun");
            compare(model.get(2, SongsModel.RoleTitle), "It's Beautiful");

            model.albumArtist = "unknown";
            waitForReady();
            compare(model.count, 0);
        }

    }
}
