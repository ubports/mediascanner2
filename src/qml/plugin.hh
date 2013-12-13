#ifndef PLUGIN_HH_
#define PLUGIN_HH_

#include <QtQml>

class MediaScannerPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    virtual void registerTypes(const char *uri) override;
};

#endif
