#include "plugin.hh"
#include "MediaFileWrapper.hh"
#include "MediaStoreWrapper.hh"

void MediaScannerPlugin::registerTypes(const char *uri) {
    qmlRegisterType<MediaStoreWrapper>(uri, 0, 1, "MediaStore");
    qmlRegisterUncreatableType<MediaFileWrapper>(uri, 0, 1, "MediaFile",
        "Use a MediaStore to retrieve MediaFiles");
}
