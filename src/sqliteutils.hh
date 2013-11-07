/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * Authors:
 *    James Henstridge <james.henstridge@canonical.com>
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

#ifndef SCAN_SQLITEUTILS_H
#define SCAN_SQLITEUTILS_H

#include <sqlite3.h>
#include <stdexcept>
#include <string>

class Statement {
public:
    Statement(sqlite3 *db, const char *sql) {
        rc = sqlite3_prepare_v2(db, sql, -1, &statement, NULL);
        if (rc != SQLITE_OK) {
            throw std::runtime_error(sqlite3_errmsg(db));
        }
    }

    ~Statement() {
        finalize();
    }

    void bind(int pos, int value) {
        rc = sqlite3_bind_int(statement, pos, value);
        if (rc != SQLITE_OK)
            throw std::runtime_error(sqlite3_errstr(rc));
    }

    void bind(int pos, const std::string &value) {
        rc = sqlite3_bind_text(statement, pos, value.c_str(), value.size(),
                               SQLITE_TRANSIENT);
        if (rc != SQLITE_OK)
            throw std::runtime_error(sqlite3_errstr(rc));
    }

    bool step() {
        rc = sqlite3_step(statement);
        switch (rc) {
        case SQLITE_DONE:
            return false;
        case SQLITE_ROW:
            return true;
        default:
            throw std::runtime_error(sqlite3_errstr(rc));
        }
    }

    std::string getText(int column) {
        if (rc != SQLITE_ROW)
            throw std::runtime_error("Statement hasn't been executed, or no more results");
        return (const char *)sqlite3_column_text(statement, column);
    }

    int getInt(int column) {
        if (rc != SQLITE_ROW)
            throw std::runtime_error("Statement hasn't been executed, or no more results");
        return sqlite3_column_int(statement, column);
    }

    void finalize() {
        if (statement != NULL) {
            rc = sqlite3_finalize(statement);
            if (rc != SQLITE_OK) {
                throw std::runtime_error("Could not finalize statement");
            }
            statement = NULL;
        }
    }

private:
    sqlite3_stmt *statement;
    int rc;
};


#endif
