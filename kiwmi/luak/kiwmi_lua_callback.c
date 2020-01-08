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

static int
l_kiwmi_lua_callback_cancel(lua_State *L)
{
    struct kiwmi_lua_callback *lc =
        *(struct kiwmi_lua_callback **)luaL_checkudata(
            L, 1, "kiwmi_lua_callback");

    wl_list_remove(&lc->listener.link);
    wl_list_remove(&lc->link);

    luaL_unref(L, LUA_REGISTRYINDEX, lc->callback_ref);

    free(lc);
    return 0;
}

static const luaL_Reg kiwmi_lua_callback_methods[] = {
    {"cancel", l_kiwmi_lua_callback_cancel},
    {NULL, NULL},
};

int
luaK_kiwmi_lua_callback_new(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TLIGHTUSERDATA); // server
    luaL_checktype(L, 2, LUA_TFUNCTION);      // callback
    luaL_checktype(L, 3, LUA_TLIGHTUSERDATA); // event_handler
    luaL_checktype(L, 4, LUA_TLIGHTUSERDATA); // signal

    struct kiwmi_lua_callback **lc_ud = lua_newuserdata(L, sizeof(*lc_ud));
    luaL_getmetatable(L, "kiwmi_lua_callback");
    lua_setmetatable(L, -2);

    struct kiwmi_lua_callback *lc = malloc(sizeof(*lc));
    if (!lc) {
        return luaL_error(L, "failed to allocate kiwmi_lua_callback");
    }

    *lc_ud = lc;

    struct kiwmi_server *server = lua_touserdata(L, 1);

    lc->server = server;

    lua_pushvalue(L, 2);
    lc->callback_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    lc->listener.notify = lua_touserdata(L, 3);
    wl_signal_add(lua_touserdata(L, 4), &lc->listener);

    wl_list_insert(&server->lua->callbacks, &lc->link);

    return 1;
}

int
luaK_kiwmi_lua_callback_register(lua_State *L)
{
    luaL_newmetatable(L, "kiwmi_lua_callback");

    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, kiwmi_lua_callback_methods, 0);

    lua_pushcfunction(L, luaK_usertype_ref_equal);
    lua_setfield(L, -2, "__eq");

    return 0;
}

void
luaK_kiwmi_lua_callback_cleanup(struct kiwmi_lua *lua)
{
    struct kiwmi_lua_callback *lc;
    struct kiwmi_lua_callback *tmp;
    wl_list_for_each_safe (lc, tmp, &lua->callbacks, link) {
        wl_list_remove(&lc->listener.link);
        wl_list_remove(&lc->link);

        luaL_unref(lua->L, LUA_REGISTRYINDEX, lc->callback_ref);

        free(lc);
    }
}
