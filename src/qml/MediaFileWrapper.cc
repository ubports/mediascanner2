#include "MediaFileWrapper.hh"

MediaFileWrapper::MediaFileWrapper(const MediaFile media, QObject *parent)
    : QObject(parent), media(media) {
}

QString MediaFileWrapper::filename() const {
    return QString::fromStdString(media.getFileName());
}

QString MediaFileWrapper::title() const {
    return QString::fromStdString(media.getTitle());
}

QString MediaFileWrapper::author() const {
    return QString::fromStdString(media.getAuthor());
}

QString MediaFileWrapper::album() const {
    return QString::fromStdString(media.getAlbum());
}
