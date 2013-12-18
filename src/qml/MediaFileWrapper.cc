#include "MediaFileWrapper.hh"

MediaFileWrapper::MediaFileWrapper(const MediaFile media, QObject *parent)
    : QObject(parent), media(media) {
}

QString MediaFileWrapper::filename() const {
    return QString::fromStdString(media.getFileName());
}

QString MediaFileWrapper::uri() const {
    return QString::fromStdString(media.getUri());
}

QString MediaFileWrapper::contentType() const {
    return QString::fromStdString(media.getContentType());
}

QString MediaFileWrapper::eTag() const {
    return QString::fromStdString(media.getETag());
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

QString MediaFileWrapper::albumArtist() const {
    return QString::fromStdString(media.getAlbumArtist());
}

QString MediaFileWrapper::date() const {
    return QString::fromStdString(media.getDate());
}

int MediaFileWrapper::trackNumber() const {
    return media.getTrackNumber();
}

int MediaFileWrapper::duration() const {
    return media.getDuration();
}
