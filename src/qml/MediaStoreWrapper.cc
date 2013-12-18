#include "MediaStoreWrapper.hh"
#include <QQmlEngine>

MediaStoreWrapper::MediaStoreWrapper(QObject *parent)
    : QObject(parent), store(MS_READ_ONLY) {
}

QList<QObject*> MediaStoreWrapper::query(const QString &q, MediaType type) {
    QList<QObject*> result;
    for (const auto &media : store.query(q.toStdString(), static_cast<::MediaType>(type))) {
        auto wrapper = new MediaFileWrapper(media);
        QQmlEngine::setObjectOwnership(wrapper, QQmlEngine::JavaScriptOwnership);
        result.append(wrapper);
    }
    return result;
}

MediaFileWrapper *MediaStoreWrapper::lookup(const QString &filename) {
    auto wrapper = new MediaFileWrapper(store.lookup(filename.toStdString()));
    QQmlEngine::setObjectOwnership(wrapper, QQmlEngine::JavaScriptOwnership);
    return wrapper;
}
