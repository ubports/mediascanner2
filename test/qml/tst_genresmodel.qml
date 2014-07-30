import QtQuick 2.0
import QtTest 1.0
import Ubuntu.MediaScanner 0.1

Item {
    id: root

    MediaStore {
        id: store
    }

    GenresModel {
        id: model
        store: store
    }

    SignalSpy {
        id: modelFilled
        target: model
        signalName: "filled"
    }

    TestCase {
        name: "GenresModelTests"

        function cleanup() {
        }

        function test_initial_state() {
            modelFilled.wait();
            compare(model.rowCount, 2);
            compare(model.get(0, ArtistsModel.RoleGenre), "rock");
            compare(model.get(1, ArtistsModel.RoleGenre), "roots");
        }

        function test_limit() {
            // The limit property is deprecated now, but we need to
            // keep it until music-app stops using it.
            compare(model.limit, -1);
            model.limit = 1;
            compare(model.limit, -1);
        }

    }
}
