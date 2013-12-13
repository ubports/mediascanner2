#include "plugin.hh"
#include "MediaFileWrapper.hh"
#include "MediaStoreWrapper.hh"

void MediaScannerPlugin::registerTypes(const char *uri) {
    qmlRegisterType<MediaStoreWrapper>(uri, 1, 0, "MediaStore");
    qmlRegisterUncreatableType<MediaFileWrapper>(uri, 1, 0, "MediaFile",
        "Use a MediaStore to retrieve MediaFiles");
}
