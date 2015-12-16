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

#include "StreamingModel.hh"

#include <cassert>
#include <exception>

#include <QEvent>
#include <QCoreApplication>
#include <QScopedPointer>
#include <QtConcurrent>

using namespace mediascanner::qml;

namespace {

const int BATCH_SIZE = 200;

class AdditionEvent : public QEvent {
private:
    std::unique_ptr<StreamingModel::RowData> rows;
    bool error = false;
    int generation;

public:
    AdditionEvent(int generation) :
        QEvent(AdditionEvent::additionEventType()), generation(generation) {
    }

    void setRows(std::unique_ptr<StreamingModel::RowData> &&r) {
        rows = std::move(r);
    }
    void setError(bool e) {
        error = e;
    }
    std::unique_ptr<StreamingModel::RowData>& getRows() { return rows; }
    bool getError() const { return error; }
    int getGeneration() const { return generation; }

    static QEvent::Type additionEventType()
    {
        static QEvent::Type type = static_cast<QEvent::Type>(QEvent::registerEventType());
        return type;
    }
};

void runQuery(int generation, StreamingModel *model, std::shared_ptr<mediascanner::MediaStoreBase> store) {
    if (!store) {
        return;
    }
    int offset = 0;
    int cursize;
    do {
        if(model->shouldWorkerStop()) {
            return;
        }
        QScopedPointer<AdditionEvent> e(new AdditionEvent(generation));
        try {
            e->setRows(model->retrieveRows(store, BATCH_SIZE, offset));
        } catch (const std::exception &exc) {
            qWarning() << "Failed to retrieve rows:" << exc.what();
            e->setError(true);
            return;
        }
        cursize = e->getRows()->size();
        if (model->shouldWorkerStop()) {
            return;
        }
        QCoreApplication::instance()->postEvent(model, e.take());
        offset += cursize;
    } while(cursize >= BATCH_SIZE);
}

}

StreamingModel::StreamingModel(QObject *parent) :
    QAbstractListModel(parent), generation(0), status(Ready) {
}

StreamingModel::~StreamingModel() {
    setWorkerStop(true);
    try {
        query_future.waitForFinished();
    } catch(...) {
        qWarning() << "Unknown error when shutting down worker thread.\n";
    }
}

void StreamingModel::updateModel() {
    if (store.isNull() || !store->store) {
        query_future = QFuture<void>();
        setStatus(Ready);
        return;
    }
    setStatus(Loading);
    setWorkerStop(false);
    query_future = QtConcurrent::run(runQuery, ++generation, this, store->store);
}

QVariant StreamingModel::get(int row, int role) const {
    return data(index(row, 0), role);
}

bool StreamingModel::event(QEvent *e) {
    if (e->type() != AdditionEvent::additionEventType()) {
        return QObject::event(e);
    }
    AdditionEvent *ae = dynamic_cast<AdditionEvent*>(e);
    assert(ae);

    // Old results may be in Qt's event queue and get delivered
    // after we have moved to a new query. Ignore them.
    if(ae->getGeneration() != generation) {
        return true;
    }

    if (ae->getError()) {
        setStatus(Error);
        return true;
    }

    auto &newrows = ae->getRows();
    bool lastBatch = newrows->size() < BATCH_SIZE;
    beginInsertRows(QModelIndex(), rowCount(), rowCount()+newrows->size()-1);
    appendRows(std::move(newrows));
    endInsertRows();
    Q_EMIT countChanged();
    if (lastBatch) {
        setStatus(Ready);
    }
    return true;
}

MediaStoreWrapper *StreamingModel::getStore() const {
    return store.data();
}

void StreamingModel::setStore(MediaStoreWrapper *store) {
    if (this->store != store) {
        if (this->store) {
            disconnect(this->store, &MediaStoreWrapper::updated,
                       this, &StreamingModel::invalidate);
        }
        this->store = store;
        if (store) {
            connect(this->store, &MediaStoreWrapper::updated,
                    this, &StreamingModel::invalidate);
        }
        invalidate();
    }
}

StreamingModel::ModelStatus StreamingModel::getStatus() const {
    return status;
}

void StreamingModel::setStatus(StreamingModel::ModelStatus status) {
    this->status = status;
    Q_EMIT statusChanged();

    if (status == Ready) {
        Q_EMIT filled();
    }
}

void StreamingModel::invalidate() {
    setWorkerStop(true);
    query_future.waitForFinished();
    beginResetModel();
    clearBacking();
    endResetModel();
    Q_EMIT countChanged();
    updateModel();
}
