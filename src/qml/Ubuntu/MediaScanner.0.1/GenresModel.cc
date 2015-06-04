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
#include <QDebug>

using namespace mediascanner::qml;

GenresModel::GenresModel(QObject *parent)
    : StreamingModel(parent) {
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

QHash<int, QByteArray> GenresModel::roleNames() const {
    return roles;
}

int GenresModel::getLimit() {
    return -1;
}

void GenresModel::setLimit(int) {
    qWarning() << "Setting limit on GenresModel is deprecated";
}

namespace {
class GenreRowData : public StreamingModel::RowData {
public:
    GenreRowData(std::vector<std::string> &&rows) : rows(std::move(rows)) {}
    ~GenreRowData() {}
    size_t size() const override { return rows.size(); }
    std::vector<std::string> rows;
};
}

std::unique_ptr<StreamingModel::RowData> GenresModel::retrieveRows(std::shared_ptr<MediaStoreBase> store, int limit, int offset) const {
    auto limit_filter = filter;
    limit_filter.setLimit(limit);
    limit_filter.setOffset(offset);
    return std::unique_ptr<StreamingModel::RowData>(
        new GenreRowData(store->listGenres(limit_filter)));
}

void GenresModel::appendRows(std::unique_ptr<StreamingModel::RowData> &&row_data) {
    GenreRowData *data = static_cast<GenreRowData*>(row_data.get());
    std::move(data->rows.begin(), data->rows.end(), std::back_inserter(results));
}

void GenresModel::clearBacking() {
    results.clear();
}
