#ifndef MEDIASCANNER_QML_MEDIASTOREWRAPPER_H
#define MEDIASCANNER_QML_MEDIASTOREWRAPPER_H

#include <QList>
#include <QObject>
#include <QString>

#include <mediascanner/MediaStore.hh>
#include "MediaFileWrapper.hh"

class MediaStoreWrapper : public QObject {
    Q_OBJECT
    Q_ENUMS(MediaType)
public:
    enum MediaType {
        Audio = AudioMedia,
        Video = VideoMedia,
        All = AllMedia,
    };
    typedef enum MediaType MediaType;
    MediaStoreWrapper(QObject *parent=0);

    Q_INVOKABLE QList<QObject*> query(const QString &q, MediaType type);
    Q_INVOKABLE MediaFileWrapper *lookup(const QString &filename);

private:
    MediaStore store;
};

#endif
