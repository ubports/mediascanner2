/*
 * Copyright (C) 2013 Canonical, Ltd.
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

#include"utils.hh"

#include<sqlite3.h>
#include<stdio.h>
#include<string>

using namespace std;

int printer(void*/*arg*/, int num_cols, char **data, char **colnames) {
    for(int i=0; i<num_cols; i++) {
        printf("%s: %s\n", colnames[i], data[i]);
    }
    return 0;
}

int main(int argc, char **argv) {
    sqlite3 *db;
    string fname = "mediastore.db";
    const char *templ = "SELECT * FROM music WHERE artist MATCH %s UNION SELECT * FROM music WHERE title MATCH %s;";
    if(argc < 2) {
        printf("%s <term>\n", argv[0]);
        return 1;
    }
    string term = argv[1];
    term += "*";
    term = sqlQuote(term);
    if(sqlite3_open_v2(fname.c_str(), &db, SQLITE_OPEN_READONLY, nullptr) != SQLITE_OK) {
        printf("%s\n", sqlite3_errmsg(db));
        return 1;
    }
    char cmd[1024];
    sprintf(cmd, templ, term.c_str(), term.c_str());
    char *err;
    if(sqlite3_exec(db, cmd, printer, NULL, &err) != SQLITE_OK) {
        printf("%s\n", sqlite3_errmsg(db));
        return 1;
    }
    sqlite3_close(db);
}
