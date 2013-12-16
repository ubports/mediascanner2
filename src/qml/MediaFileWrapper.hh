#ifndef MEDIASCANNER_QML_MEDIAFILEWRAPPER_H
#define MEDIASCANNER_QML_MEDIAFILEWRAPPER_H

#include <QObject>
#include <QString>

#include <mediascanner/MediaFile.hh>

class MediaFileWrapper : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString filename READ filename CONSTANT)
    Q_PROPERTY(QString title READ title CONSTANT)
    Q_PROPERTY(QString author READ author CONSTANT)
    Q_PROPERTY(QString album READ album CONSTANT)
public:
    MediaFileWrapper(MediaFile media, QObject *parent=0);
    QString filename() const;
    QString title() const;
    QString author() const;
    QString album() const;

private:
    const MediaFile media;
};

#endif
