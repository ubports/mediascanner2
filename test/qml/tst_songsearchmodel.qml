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
        id: modelStatus
        target: model
        signalName: "statusChanged"
    }

    TestCase {
        name: "SongsSearchModelTests"

        function waitForReady() {
            while (model.status == SongsSearchModel.Loading) {
                modelStatus.wait();
            }
            compare(model.status, SongsSearchModel.Ready);
        }

        function test_search() {
            // By default, the model lists all songs.
            waitForReady();
            compare(model.count, 7, "songs_model.count == 7");

            model.query = "revolution";
            waitForReady();
            compare(model.count, 1, "songs_model.count == 1");
            compare(model.get(0, SongsSearchModel.RoleTitle), "Revolution");
        }
    }
}
