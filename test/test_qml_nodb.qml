import QtQuick 2.0
import QtTest 1.0
import Ubuntu.MediaScanner 0.1

TestCase {
    id: root
    name: "NoDatabaseTests"

    function test_mediastore() {
        ignoreWarning("Could not initialise media store: unable to open database file");
        var store = Qt.createQmlObject(
            "import Ubuntu.MediaScanner 0.1;" +
            "MediaStore {}", root);
        if (store === null) {
            fail("Could not create MediaStore component");
        }

        ignoreWarning("query() called on invalid MediaStore");
        compare(store.query("foo", MediaStore.AllMedia), []);

        ignoreWarning("lookup() called on invalid MediaStore");
        compare(store.lookup("/some/file"), null);
    }

    function test_songsmodel() {
        ignoreWarning("Could not initialise media store: unable to open database file");
        var model = Qt.createQmlObject(
            "import Ubuntu.MediaScanner 0.1;" +
            "SongsModel {" +
            "  store: MediaStore {}" +
            "}", root);
        if (model === null) {
            fail("Could not create SongsModel component");
        }
        // Model is empty
        compare(model.count, 0);
    }
}
