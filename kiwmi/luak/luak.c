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

#include "luak/kiwmi_lua_callback.h"
#include "luak/kiwmi_server.h"
#include "luak/kiwmi_view.h"

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
        return NULL;
    }

    luaL_openlibs(L);

    // register types
    int error = 0;

    lua_pushcfunction(L, luaK_kiwmi_lua_callback_register);
    error |= lua_pcall(L, 0, 0, 0);
    lua_pushcfunction(L, luaK_kiwmi_view_register);
    error |= lua_pcall(L, 0, 0, 0);
    lua_pushcfunction(L, luaK_kiwmi_server_register);
    error |= lua_pcall(L, 0, 0, 0);

    if (error) {
        wlr_log(WLR_ERROR, "%s", lua_tostring(L, -1));
        return NULL;
    }

    // create kiwmi global
    lua_pushcfunction(L, luaK_kiwmi_server_new);
    lua_pushlightuserdata(L, server);
    if (lua_pcall(L, 1, 1, 0)) {
        wlr_log(WLR_ERROR, "%s", lua_tostring(L, -1));
        return NULL;
    }
    lua_setglobal(L, "kiwmi");

    lua->L = L;

    wl_list_init(&lua->callbacks);

    return lua;
}

bool
luaK_dofile(struct kiwmi_lua *lua, const char *config_path)
{
    if (luaL_dofile(lua->L, config_path)) {
        wlr_log(
            WLR_ERROR, "Error running config: %s", lua_tostring(lua->L, -1));
        return false;
    }

    return true;
}

void
luaK_destroy(struct kiwmi_lua *lua)
{
    luaK_kiwmi_lua_callback_cleanup(lua);

    lua_close(lua->L);

    free(lua);
}
