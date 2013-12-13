#ifndef MEDIASCANNER_QML_MEDIASTOREWRAPPER_H
#define MEDIASCANNER_QML_MEDIASTOREWRAPPER_H

#include <QList>
#include <QObject>
#include <QString>

#include <mediascanner/MediaStore.hh>
#include "MediaFileWrapper.hh"

class MediaStoreWrapper : public QObject {
    Q_OBJECT
public:
    MediaStoreWrapper();

    Q_INVOKABLE QList<MediaFileWrapper*> query(const QString &q);
    Q_INVOKABLE MediaFileWrapper *lookup(const QString &filename);

private:
    MediaStore store;
};

#endif
