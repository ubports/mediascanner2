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

    TestCase {
        name: "GenresModelTests"

        function cleanup() {
            model.limit = -1;
        }

        function test_initial_state() {
            compare(model.limit, -1);

            compare(model.rowCount, 2);
            compare(model.get(0, ArtistsModel.RoleGenre), "rock");
            compare(model.get(1, ArtistsModel.RoleGenre), "roots");
        }

        function test_limit() {
            model.limit = 1;
            compare(model.rowCount, 1);

            model.limit = 42;
            compare(model.rowCount, 2);

            model.limit = -1;
            compare(model.rowCount, 2);
        }

    }
}
