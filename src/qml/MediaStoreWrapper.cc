#include "MediaStoreWrapper.hh"

MediaStoreWrapper::MediaStoreWrapper(QObject *parent)
    : QObject(parent), store(MS_READ_ONLY) {
}

QList<QObject*> MediaStoreWrapper::query(const QString &q, MediaType type) {
    QList<QObject*> result;
    for (const auto &media : store.query(q.toStdString(), static_cast<::MediaType>(type))) {
        result.append(new MediaFileWrapper(media));
    }
    return result;
}

MediaFileWrapper *MediaStoreWrapper::lookup(const QString &filename) {
    return new MediaFileWrapper(store.lookup(filename.toStdString()));
}
