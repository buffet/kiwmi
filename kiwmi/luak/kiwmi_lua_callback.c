/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "luak/kiwmi_lua_callback.h"

#include <stdlib.h>

#include <lauxlib.h>
#include <wayland-server.h>

int
luaK_kiwmi_lua_callback_new(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TLIGHTUSERDATA); // server
    luaL_checktype(L, 2, LUA_TFUNCTION);      // callback
    luaL_checktype(L, 3, LUA_TLIGHTUSERDATA); // event_handler
    luaL_checktype(L, 4, LUA_TLIGHTUSERDATA); // signal
    luaL_checktype(L, 5, LUA_TLIGHTUSERDATA); // object

    struct kiwmi_lua_callback *lc = malloc(sizeof(*lc));
    if (!lc) {
        return luaL_error(L, "failed to allocate kiwmi_lua_callback");
    }

    struct kiwmi_server *server = lua_touserdata(L, 1);

    lc->server = server;

    lua_pushvalue(L, 2);
    lc->callback_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    lc->listener.notify = lua_touserdata(L, 3);
    wl_signal_add(lua_touserdata(L, 4), &lc->listener);

    struct kiwmi_object *object = lua_touserdata(L, 5);

    wl_list_insert(&object->callbacks, &lc->link);

    return 0;
}
