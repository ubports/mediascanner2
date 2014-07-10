import QtQuick 2.0
import QtTest 1.0
import Ubuntu.MediaScanner 0.1

Item {
    id: root

    MediaStore {
        id: store
    }

    SongsSearchModel {
        id: songs_model
        store: store
        query: ""
    }

    SignalSpy {
        id: modelFilled
        target: model
        signalName: "filled"
    }

    TestCase {
        name: "SongsSearchModelTests"
        function test_search() {
            songs_model.query = "revolution";
            modelFilled.wait();
            compare(songs_model.rowCount, 1, "songs_model.rowCount == 1");
            compare(songs_model.get(0, SongsSearchModel.RoleTitle), "Revolution");

            // By default, the model lists all songs.
            songs_model.query = "";
            modelFilled.wait();
            compare(songs_model.rowCount, 7, "songs_model.rowCount == 7");
        }
    }
}
