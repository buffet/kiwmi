/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "luak/luak.h"

#include <stdlib.h>

#include <lauxlib.h>
#include <lualib.h>
#include <wlr/util/log.h>

#include "luak/ipc.h"
#include "luak/kiwmi_keyboard.h"
#include "luak/kiwmi_lua_callback.h"
#include "luak/kiwmi_output.h"
#include "luak/kiwmi_server.h"
#include "luak/kiwmi_view.h"

int
luaK_callback_register_dispatch(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TUSERDATA); // server
    luaL_checktype(L, 2, LUA_TSTRING);   // type
    luaL_checktype(L, 3, LUA_TFUNCTION); // callback

    int has_mt = lua_getmetatable(L, 1);
    luaL_argcheck(L, has_mt, 1, "no metatable");
    lua_getfield(L, -1, "__events");
    luaL_argcheck(L, lua_istable(L, -1), 1, "no __events");
    lua_pushvalue(L, 2);
    lua_gettable(L, -2);

    luaL_argcheck(L, lua_iscfunction(L, -1), 2, "invalid event");
    lua_pushvalue(L, 1);
    lua_pushvalue(L, 3);

    if (lua_pcall(L, 2, 1, 0)) {
        wlr_log(WLR_ERROR, "%s", lua_tostring(L, -1));
        return 0;
    }

    return 1;
}

struct kiwmi_lua *
luaK_create(struct kiwmi_server *server)
{
    struct kiwmi_lua *lua = malloc(sizeof(*lua));
    if (!lua) {
        wlr_log(WLR_ERROR, "Failed to allocate kiwmi_lua");
        return NULL;
    }

    lua_State *L = luaL_newstate();
    if (!L) {
        free(lua);
        return NULL;
    }

    luaL_openlibs(L);

    // register types
    int error = 0;

    lua_pushcfunction(L, luaK_kiwmi_keyboard_register);
    error |= lua_pcall(L, 0, 0, 0);
    lua_pushcfunction(L, luaK_kiwmi_lua_callback_register);
    error |= lua_pcall(L, 0, 0, 0);
    lua_pushcfunction(L, luaK_kiwmi_output_register);
    error |= lua_pcall(L, 0, 0, 0);
    lua_pushcfunction(L, luaK_kiwmi_server_register);
    error |= lua_pcall(L, 0, 0, 0);
    lua_pushcfunction(L, luaK_kiwmi_view_register);
    error |= lua_pcall(L, 0, 0, 0);

    if (error) {
        wlr_log(WLR_ERROR, "%s", lua_tostring(L, -1));
        lua_close(L);
        free(lua);
        return NULL;
    }

    // create kiwmi global
    lua_pushcfunction(L, luaK_kiwmi_server_new);
    lua_pushlightuserdata(L, server);
    if (lua_pcall(L, 1, 1, 0)) {
        wlr_log(WLR_ERROR, "%s", lua_tostring(L, -1));
        lua_close(L);
        free(lua);
        return NULL;
    }
    lua_setglobal(L, "kiwmi");

    lua->L = L;

    wl_list_init(&lua->callbacks);

    if (!luaK_ipc_init(server, lua)) {
        wlr_log(WLR_ERROR, "Failed to initialize IPC");
        lua_close(L);
        free(lua);
        return NULL;
    }

    return lua;
}

bool
luaK_dofile(struct kiwmi_lua *lua, const char *config_path)
{
    int top = lua_gettop(lua->L);

    if (luaL_dofile(lua->L, config_path)) {
        wlr_log(
            WLR_ERROR, "Error running config: %s", lua_tostring(lua->L, -1));
        return false;
    }

    lua_pop(lua->L, top - lua_gettop(lua->L));

    return true;
}

void
luaK_destroy(struct kiwmi_lua *lua)
{
    luaK_kiwmi_lua_callback_cleanup(lua);

    lua_close(lua->L);

    free(lua);
}
