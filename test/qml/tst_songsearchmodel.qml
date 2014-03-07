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

    ListView {
        id: songs_view
        model: songs_model
    }

    TestCase {
        name: "SongsSearchModelTests"
        function test_foo() {
        }
    }
}
