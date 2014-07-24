import QtQuick 2.0
import QtTest 1.0
import Ubuntu.MediaScanner 0.1

Item {
    id: root

    MediaStore {
        id: store
    }

    SongsSearchModel {
        id: model
        store: store
    }

    SignalSpy {
        id: modelFilled
        target: model
        signalName: "filled"
    }

    TestCase {
        name: "SongsSearchModelTests"
        function test_search() {
            // By default, the model lists all songs.
            modelFilled.wait();
            compare(model.rowCount, 7, "songs_model.rowCount == 7");

            model.query = "revolution";
            modelFilled.wait();
            compare(model.rowCount, 1, "songs_model.rowCount == 1");
            compare(model.get(0, SongsSearchModel.RoleTitle), "Revolution");
        }
    }
}
