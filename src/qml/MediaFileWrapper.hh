#ifndef MEDIASCANNER_QML_MEDIAFILEWRAPPER_H
#define MEDIASCANNER_QML_MEDIAFILEWRAPPER_H

#include <QObject>
#include <QString>

#include <mediascanner/MediaFile.hh>

class MediaFileWrapper : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString filename READ filename CONSTANT)
public:
    MediaFileWrapper(MediaFile media);
    QString filename() const;

private:
    const MediaFile media;
};

#endif
