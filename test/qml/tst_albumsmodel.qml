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
        id: modelStatus
        target: model
        signalName: "statusChanged"
    }

    TestCase {
        name: "AlbumsModelTests"

        function waitForReady() {
            while (model.status == AlbumsModel.Loading) {
                modelStatus.wait();
            }
            compare(model.status, AlbumsModel.Ready);
        }

        function cleanup() {
            model.artist = undefined;
            model.albumArtist = undefined;
            model.genre = undefined;
            waitForReady();
        }

        function test_initial_state() {
            compare(model.artist, undefined);
            compare(model.albumArtist, undefined);
            compare(model.genre, undefined);

            compare(model.count, 4);
            compare(model.get(0, AlbumsModel.RoleTitle), "Ivy and the Big Apples");
            compare(model.get(0, AlbumsModel.RoleArtist), "Spiderbait");
            compare(model.get(0, AlbumsModel.RoleDate), "1996-10-04");
            compare(model.get(0, AlbumsModel.RoleGenre), "rock");
            compare(model.get(0, AlbumsModel.RoleArt), "image://albumart/artist=Spiderbait&album=Ivy%20and%20the%20Big%20Apples");

            compare(model.get(1, AlbumsModel.RoleTitle), "Spiderbait");
            compare(model.get(1, AlbumsModel.RoleArtist), "Spiderbait");
            compare(model.get(1, AlbumsModel.RoleDate), "2013-11-15");
            compare(model.get(1, AlbumsModel.RoleGenre), "rock");
            compare(model.get(1, AlbumsModel.RoleArt), "image://albumart/artist=Spiderbait&album=Spiderbait");

            compare(model.get(2, AlbumsModel.RoleTitle), "April Uprising");
            compare(model.get(2, AlbumsModel.RoleArtist), "The John Butler Trio");
            compare(model.get(2, AlbumsModel.RoleDate), "2010-01-01");
            compare(model.get(2, AlbumsModel.RoleGenre), "roots");
            compare(model.get(2, AlbumsModel.RoleArt), "image://albumart/artist=The%20John%20Butler%20Trio&album=April%20Uprising");

            compare(model.get(3, AlbumsModel.RoleTitle), "Sunrise Over Sea");
            compare(model.get(3, AlbumsModel.RoleArtist), "The John Butler Trio");
            compare(model.get(3, AlbumsModel.RoleDate), "2004-03-08");
            compare(model.get(3, AlbumsModel.RoleGenre), "roots");
            compare(model.get(3, AlbumsModel.RoleArt), "image://albumart/artist=The%20John%20Butler%20Trio&album=Sunrise%20Over%20Sea");
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
            compare(model.count, 2);

            compare(model.get(0, AlbumsModel.RoleTitle), "April Uprising");
            compare(model.get(0, AlbumsModel.RoleArtist), "The John Butler Trio");

            model.artist = "unknown";
            waitForReady();
            compare(model.count, 0);
        }

        function test_album_artist() {
            model.albumArtist = "The John Butler Trio";
            waitForReady();
            compare(model.count, 2);

            compare(model.get(0, AlbumsModel.RoleTitle), "April Uprising");
            compare(model.get(0, AlbumsModel.RoleArtist), "The John Butler Trio");

            model.albumArtist = "unknown";
            waitForReady();
            compare(model.count, 0);
        }

        function test_genre() {
            model.genre = "rock";
            waitForReady();
            compare(model.count, 2);
            compare(model.get(0, AlbumsModel.RoleTitle), "Ivy and the Big Apples");
            compare(model.get(1, AlbumsModel.RoleTitle), "Spiderbait");

            model.genre = "unknown";
            waitForReady();
            compare(model.count, 0);
        }

    }
}
