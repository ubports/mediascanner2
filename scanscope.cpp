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

#define CATEGORY_ICON_PATH "/usr/share/unity/icons/preview_play.svg"
#define GROUP_NAME "com.canonical.Unity.Scope.Music.Localscan"
#define UNIQUE_NAME "/com/canonical/unity/scope/Music/localscan"

#include<unity.h>
#include<sqlite3.h>
#include<string>
#include<vector>

sqlite3 *db;

struct res {
    std::string filename;
    std::string title;
    std::string artist;
    std::string album;
};

int storer(void* arg, int /*num_cols*/, char **data, char** /*colnames*/) {
    std::vector<res> *store = reinterpret_cast<std::vector<res> *> (arg);
    res r;
    r.filename = data[0];
    r.title = data[1];
    r.artist = data[2];
    r.album = data[3];
    store->push_back(r);
    return 0;
}

static void search_func(UnityScopeSearchBase* search, void* /*user_data*/) {
    GHashTable *metadata = NULL;
    UnityScopeResult scope_result = { 0, };
    std::vector<res> matches;
    const char *templ = "SELECT * FROM music WHERE artist MATCH '%s*' UNION SELECT * FROM music WHERE title MATCH '%s*' ";
    char cmd[1024];
    printf("Query: %s\n", search->search_context->search_query);
    sprintf(cmd, templ, search->search_context->search_query, search->search_context->search_query);
    char *err;
    if(sqlite3_exec(db, cmd, storer, &matches, &err) != SQLITE_OK) {
        fprintf(stderr, "%s\n", sqlite3_errmsg(db));
        return;
    }

    /* Iterate through the returned results and add them to the
     * Unity's result set
     */
    for (const auto &m: matches) {

        /* Build and populate a scope result from the source data */
        std::string uri = std::string("file://") + m.filename;
        scope_result.uri = (gchar*)uri.c_str();
        scope_result.title = (gchar*)m.title.c_str();
        //scope_result.icon_hint = result->icon_url;
        scope_result.category = 0;
        scope_result.result_type = UNITY_RESULT_TYPE_DEFAULT;
        scope_result.mimetype = "audio/mp3";
        //scope_result.comment = result->description;
        //scope_result.dnd_uri = result->link;

        /* Insert the metadata, if available */
        metadata = g_hash_table_new(g_str_hash, g_str_equal);
        if(!m.artist.empty()) {
            GVariant *var = g_variant_new_string((const gchar*)m.artist.c_str());
            g_hash_table_insert(metadata, (gpointer)"author", (gpointer)var);
        }
        /*
        if (result->creation_date) {
             g_hash_table_insert(metadata, "creation_date",
                     g_variant_new_string(result->creation_date));
        }
        */
        scope_result.metadata = metadata;

         /*
          * Add the returned result to the search results list, taking a
          * copy of the data passed in via scope_result
          */
        unity_result_set_add_result(search->search_context->result_set,
                                     &scope_result);
        g_hash_table_unref(metadata);
     }

}

static UnityAbstractPreview* preview_func(UnityResultPreviewer *previewer, void *user_data) {
    return nullptr;
}

int main(void) {
    std::string fname = "mediastore.db";
    UnitySimpleScope *scope = NULL;
    UnityScopeDBusConnector *connector = NULL;
    UnityCategorySet *cats = NULL;
    UnityCategory *cat = NULL;
    GIcon *icon = NULL;

    if(sqlite3_open_v2(fname.c_str(), &db, SQLITE_OPEN_READONLY, nullptr) != SQLITE_OK) {
        fprintf(stderr, "%s\n", sqlite3_errmsg(db));
        return 1;
    }
    /* Create and set a category for the scope, including an icon */
    icon = g_themed_icon_new(CATEGORY_ICON_PATH);

    cat = unity_category_new("global", "Openclipart", icon,
                             UNITY_CATEGORY_RENDERER_HORIZONTAL_TILE);
    cats = unity_category_set_new();
    unity_category_set_add(cats, cat);

    /* Create and set up the scope */
    scope = unity_simple_scope_new();
    unity_simple_scope_set_group_name(scope, GROUP_NAME);
    unity_simple_scope_set_unique_name(scope, UNIQUE_NAME);
    unity_simple_scope_set_search_func(scope, search_func, NULL, NULL);
    unity_simple_scope_set_preview_func(scope, preview_func, NULL, NULL);
    unity_simple_scope_set_category_set(scope, cats);

    g_object_unref (icon);
    unity_object_unref (cat);
    unity_object_unref (cats);

    /*
     * Setting up the connector is an action that will not be required
     * in future revisions of the API. In particular, we only need it here
     * since the scope is running locally on the device as opposed to
     * running on the Smart Scopes server
     */
    connector = unity_scope_dbus_connector_new(UNITY_ABSTRACT_SCOPE(scope));
    unity_scope_dbus_connector_export(connector, NULL);
    unity_scope_dbus_connector_run();

    sqlite3_close(db);
    return 0;
}

