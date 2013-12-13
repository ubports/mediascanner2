#include "MediaFileWrapper.hh"

MediaFileWrapper::MediaFileWrapper(const MediaFile media) : media(media) {
}

QString MediaFileWrapper::filename() const {
    return QString::fromStdString(media.getFileName());
}
