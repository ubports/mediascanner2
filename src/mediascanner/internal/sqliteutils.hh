/*
 * Copyright (C) 2013 Canonical, Ltd.
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

#ifndef SCAN_SQLITEUTILS_H
#define SCAN_SQLITEUTILS_H

#include <sqlite3.h>
#include <stdexcept>
#include <string>

namespace mediascanner {

class Statement {
public:
    Statement(sqlite3 *db, const char *sql) {
        rc = sqlite3_prepare_v2(db, sql, -1, &statement, nullptr);
        if (rc != SQLITE_OK) {
            throw std::runtime_error(sqlite3_errmsg(db));
        }
    }

    ~Statement() {
        try {
            finalize();
        } catch(const std::exception &e) {
            fprintf(stderr, "Error finalising statement: %s\n", e.what());
        } catch(...) {
            fprintf(stderr, "Unknown error finalising statement.\n");
        }
    }

    void bind(int pos, int value) {
        rc = sqlite3_bind_int(statement, pos, value);
        if (rc != SQLITE_OK)
            throw std::runtime_error(sqlite3_errstr(rc));
    }

    void bind(int pos, double value) {
        rc = sqlite3_bind_double(statement, pos, value);
        if (rc != SQLITE_OK)
            throw std::runtime_error(sqlite3_errstr(rc));
    }

    void bind(int pos, const std::string &value) {
        rc = sqlite3_bind_text(statement, pos, value.c_str(), value.size(),
                               SQLITE_TRANSIENT);
        if (rc != SQLITE_OK)
            throw std::runtime_error(sqlite3_errstr(rc));
    }

    void bind(int pos, void *blob, int length) {
        rc = sqlite3_bind_blob(statement, pos, blob, length, SQLITE_STATIC);
        if (rc != SQLITE_OK)
            throw std::runtime_error(sqlite3_errstr(rc));
    }

    bool step() {
        // Sqlite docs list a few cases where you need to to a rollback
        // if a calling step fails. We don't match those cases but if
        // we change queries that may start to happen.
        // https://sqlite.org/c3ref/step.html
        //
        // The proper fix would probably be to move to a WAL log, but
        // it seems to require write access to the mediastore dir
        // even for readers, which is problematic for confined apps.
        int retry_count=0;
        const int max_retries = 100;
        do {
            rc = sqlite3_step(statement);
            retry_count++;
        } while(rc == SQLITE_BUSY && retry_count < max_retries);
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

    double getDouble(int column) {
        if (rc != SQLITE_ROW)
            throw std::runtime_error("Statement hasn't been executed, or no more results");
        return sqlite3_column_double(statement, column);
    }

    void finalize() {
        if (statement != nullptr) {
            rc = sqlite3_finalize(statement);
            if (rc != SQLITE_OK) {
                std::string msg("Could not finalize statement: ");
                msg += sqlite3_errstr(rc);
                throw std::runtime_error(msg);
            }
            statement = nullptr;
        }
    }

private:
    sqlite3_stmt *statement;
    int rc;
};

}

#endif
