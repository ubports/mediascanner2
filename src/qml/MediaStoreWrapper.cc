#include "MediaStoreWrapper.hh"

MediaStoreWrapper::MediaStoreWrapper() : store(MS_READ_ONLY) {
}

QList<MediaFileWrapper*> MediaStoreWrapper::query(const QString &q) {
    QList<MediaFileWrapper*> result;
    for (const auto &media : store.query(q.toStdString(), AllMedia)) {
        result.append(new MediaFileWrapper(media));
    }
    return result;
}

MediaFileWrapper *MediaStoreWrapper::lookup(const QString &filename) {
    return new MediaFileWrapper(store.lookup(filename.toStdString()));
}
