#include "MediaFileWrapper.hh"

MediaFileWrapper::MediaFileWrapper(const MediaFile media, QObject *parent)
    : QObject(parent), media(media) {
}

QString MediaFileWrapper::filename() const {
    return QString::fromStdString(media.getFileName());
}
