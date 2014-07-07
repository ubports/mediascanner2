/*
 * Copyright (C) 2014 Canonical, Ltd.
 *
 * Authors:
 *    James Henstridge <james.henstridge@canonical.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "GenresModel.hh"
#include <exception>
#include <QDebug>

using namespace mediascanner::qml;

GenresModel::GenresModel(QObject *parent)
    : QAbstractListModel(parent),
      store(nullptr) {
    roles[Roles::RoleGenre] = "genre";
}

int GenresModel::rowCount(const QModelIndex &) const {
    return results.size();
}

QVariant GenresModel::data(const QModelIndex &index, int role) const {
    if (index.row() < 0 || index.row() >= (ptrdiff_t)results.size()) {
        return QVariant();
    }
    switch (role) {
    case RoleGenre:
        return QString::fromStdString(results[index.row()]);
    default:
        return QVariant();
    }
}

QVariant GenresModel::get(int row, GenresModel::Roles role) const {
    return data(index(row, 0), role);
}

QHash<int, QByteArray> GenresModel::roleNames() const {
    return roles;
}

MediaStoreWrapper *GenresModel::getStore() {
    return store;
}

void GenresModel::setStore(MediaStoreWrapper *store) {
    if (this->store != store) {
        this->store = store;
        update();
    }
}

int GenresModel::getLimit() {
    return filter.getLimit();
}

void GenresModel::setLimit(int limit) {
    if (filter.getLimit() != limit) {
        filter.setLimit(limit);
        update();
    }
}

void GenresModel::update() {
    beginResetModel();
    if (store == nullptr) {
        this->results.clear();
    } else {
        try {
            this->results = store->store->listGenres(filter);
        } catch (const std::exception &e) {
            qWarning() << "Failed to retrieve genre list:" << e.what();
            this->results.clear();
        }
    }
    endResetModel();
    Q_EMIT rowCountChanged();
}
