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

static void search_func(UnityScopeSearchBase* search, void* user_data) {
}

static UnityAbstractPreview* preview_func(UnityResultPreviewer *previewer, void *user_data) {
}

int main(void) {
    UnitySimpleScope *scope = NULL;
    UnityScopeDBusConnector *connector = NULL;
    UnityCategorySet *cats = NULL;
    UnityCategory *cat = NULL;
    GIcon *icon = NULL;

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

    return 0;
}

