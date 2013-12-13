#include <memory>

#include <QList>
#include <QObject>
#include <QString>
#include <QtQml>

#include <mediascanner/MediaFile.hh>
#include <mediascanner/MediaStore.hh>

class MediaFileWrapper : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString filename READ filename)
public:
    MediaFileWrapper(MediaFile media) : media(media) {}

    QString filename() const {
        return QString::fromStdString(media.getFileName());
    }

private:
    const MediaFile media;
};

class MediaStoreWrapper : public QObject {
    Q_OBJECT
public:
    MediaStoreWrapper() : store(MS_READ_ONLY) {}

    Q_INVOKABLE QList<MediaFileWrapper*> query(const QString &q) {
        QList<MediaFileWrapper*> result;
        for (const auto &media : store.query(q.toStdString(), AllMedia)) {
            result.append(new MediaFileWrapper(media));
        }
        return result;
    }

    Q_INVOKABLE MediaFileWrapper *lookup(const QString &filename) {
        return new MediaFileWrapper(store.lookup(filename.toStdString()));
    }


private:
    MediaStore store;
};

class MediaScannerPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    virtual void registerTypes(const char *uri) override {
        qmlRegisterType<MediaStoreWrapper>(uri, 1, 0, "MediaStore");
        qmlRegisterUncreatableType<MediaFileWrapper>(uri, 1, 0, "MediaFile",
            "Use a MediaStore to retrieve MediaFiles");
    }
};
