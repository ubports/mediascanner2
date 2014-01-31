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

#include <gtest/gtest.h>
#include <mediascanner/internal/sqliteutils.hh>

using namespace std;
using namespace mediascanner;

class SqliteTest : public ::testing::Test {
public:
    SqliteTest() : db(NULL) {
    }

    virtual void SetUp() {
        int rc = sqlite3_open(":memory:", &db);
        if (rc != SQLITE_OK) {
            throw runtime_error(sqlite3_errstr(rc));
        }
    }

    virtual void TearDown() {
        if (db != NULL) {
            int rc = sqlite3_close(db);
            if (rc != SQLITE_OK) {
                throw runtime_error(sqlite3_errstr(rc));
            }
            db = NULL;
        }
    }

    sqlite3 *db;
};

TEST_F(SqliteTest, Execute) {
    Statement stmt(db, "SELECT 1");
    EXPECT_EQ(true, stmt.step());
    stmt.finalize();
}

TEST_F(SqliteTest, GetInt) {
    Statement stmt(db, "SELECT 42");
    EXPECT_EQ(true, stmt.step());
    EXPECT_EQ(42, stmt.getInt(0));
    stmt.finalize();
}

TEST_F(SqliteTest, GetText) {
    Statement stmt(db, "SELECT 'foo'");
    EXPECT_EQ(true, stmt.step());
    EXPECT_EQ("foo", stmt.getText(0));
    stmt.finalize();
}

TEST_F(SqliteTest, BindInt) {
    Statement stmt(db, "SELECT ?");
    stmt.bind(1, 42);
    EXPECT_EQ(true, stmt.step());
    EXPECT_EQ(42, stmt.getInt(0));
    stmt.finalize();
}

TEST_F(SqliteTest, BindText) {
    Statement stmt(db, "SELECT ?");
    stmt.bind(1, "foo");
    EXPECT_EQ(true, stmt.step());
    EXPECT_EQ("foo", stmt.getText(0));
    stmt.finalize();
}

TEST_F(SqliteTest, Insert) {
    Statement create(db, "CREATE TABLE foo (id INT PRIMARY KEY)");
    EXPECT_FALSE(create.step());
    create.finalize();

    Statement insert(db, "INSERT INTO foo(id) VALUES (?)");
    insert.bind(1, 42);
    EXPECT_FALSE(insert.step());
    insert.finalize();

    Statement select(db, "SELECT id FROM foo");
    EXPECT_TRUE(select.step());
    EXPECT_EQ(42, select.getInt(0));
    EXPECT_FALSE(select.step());
    select.finalize();
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
