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

    TestCase {
        name: "SongsSearchModelTests"
        function test_search() {
            // By default, the model lists all songs.
            compare(songs_model.rowCount, 7, "songs_model.rowCount == 7");
            songs_model.query = "revolution";
            compare(songs_model.rowCount, 1, "songs_model.rowCount == 1");
        }
    }
}
