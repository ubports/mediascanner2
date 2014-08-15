/*
 * Copyright (C) 2014 Canonical, Ltd.
 *
 * Authors:
 *    Jussi Pakkanen <jussi.pakkanen@canonical.com>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of version 3 of the GNU General Public License as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MEDIASCANNER_QML_STREAMINGMODEL_H
#define MEDIASCANNER_QML_STREAMINGMODEL_H

#include <atomic>
#include <memory>

#include <QAbstractListModel>
#include <QFuture>
#include <QPointer>

#include "MediaStoreWrapper.hh"

namespace mediascanner {
namespace qml {

class StreamingModel : public QAbstractListModel {
    Q_OBJECT
    Q_ENUMS(ModelStatus)
    Q_PROPERTY(mediascanner::qml::MediaStoreWrapper* store READ getStore WRITE setStore)
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
    Q_PROPERTY(int rowCount READ rowCount NOTIFY countChanged)
    Q_PROPERTY(ModelStatus status READ getStatus NOTIFY statusChanged)
public:
    enum ModelStatus {
        Ready,
        Loading,
        Error
    };

    explicit StreamingModel(QObject *parent=nullptr);
    virtual ~StreamingModel();

    Q_INVOKABLE QVariant get(int row, int role) const;

    bool event(QEvent *e) override;
    bool shouldWorkerStop() const noexcept { return stopflag.load(std::memory_order_consume); }

    // Subclasses should implement the following, along with rowCount and data
    class RowData {
    public:
        virtual ~RowData() {}
        virtual size_t size() const = 0;
    };
    virtual std::unique_ptr<RowData> retrieveRows(std::shared_ptr<mediascanner::MediaStoreBase> store, int limit, int offset) const = 0;
    virtual void appendRows(std::unique_ptr<RowData> &&row_data) = 0;
    virtual void clearBacking() = 0;

protected:
    MediaStoreWrapper *getStore() const;
    void setStore(MediaStoreWrapper *store);

    ModelStatus getStatus() const;
    void setStatus(ModelStatus status);

private:
    void updateModel();
    void setWorkerStop(bool new_stop_status) noexcept { stopflag.store(new_stop_status, std::memory_order_release); }

    QPointer<MediaStoreWrapper> store;
    QFuture<void> query_future;
    int generation;
    std::atomic<bool> stopflag;
    ModelStatus status;

Q_SIGNALS:
    void countChanged();
    void statusChanged();
    // This next signal is here for backwards compatibility
    void filled();

public Q_SLOTS:

    void invalidate();
};

}
}

#endif
