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

#include"InvalidationSender.hh"
#include<string>
#include<cstdlib>
#include<cstdio>

using namespace std;

InvalidationSender::InvalidationSender() {

}

void InvalidationSender::invalidate() {
    string invocation("dbus-send /com/canonical/unity/scopes ");
    invocation += "com.canonical.unity.scopes.InvalidateResults string:";
    const string m_invoc = invocation + "mediascanner-music";
    const string v_invoc = invocation + "mediascanner-video";
    if(system(m_invoc.c_str()) != 0) {
        fprintf(stderr, "Could not invalidate music scope results.\n");
    }
    if(system(v_invoc.c_str()) != 0) {
        fprintf(stderr, "Could not invalidate video scope results.\n");
    }
}
