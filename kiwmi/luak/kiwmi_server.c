/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "luak/kiwmi_server.h"

#include <lauxlib.h>
#include <wayland-server.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/util/log.h>

#include "desktop/view.h"
#include "input/cursor.h"
#include "luak/kiwmi_lua_callback.h"
#include "luak/kiwmi_view.h"
#include "server.h"

static int
l_kiwmi_server_quit(lua_State *L)
{
    struct kiwmi_server *server =
        *(struct kiwmi_server **)luaL_checkudata(L, 1, "kiwmi_server");

    wl_display_terminate(server->wl_display);

    return 0;
}

static int
l_kiwmi_server_view_under_cursor(lua_State *L)
{
    struct kiwmi_server *server =
        *(struct kiwmi_server **)luaL_checkudata(L, 1, "kiwmi_server");

    struct kiwmi_cursor *cursor = server->input.cursor;

    struct wlr_surface *surface;
    double sx;
    double sy;

    struct kiwmi_view *view = view_at(
        &server->desktop,
        cursor->cursor->x,
        cursor->cursor->y,
        &surface,
        &sx,
        &sy);

    if (view) {
        lua_pushcfunction(L, luaK_kiwmi_view_new);
        lua_pushlightuserdata(L, view);
        if (lua_pcall(L, 1, 1, 0)) {
            wlr_log(WLR_ERROR, "%s", lua_tostring(L, -1));
            return 0;
        }
    } else {
        lua_pushnil(L);
    }

    return 1;
}

static const luaL_Reg kiwmi_server_methods[] = {
    {"on", luaK_callback_register_dispatch},
    {"quit", l_kiwmi_server_quit},
    {"view_under_cursor", l_kiwmi_server_view_under_cursor},
    {NULL, NULL},
};

static void
kiwmi_server_on_view_notify(struct wl_listener *listener, void *data)
{
    struct kiwmi_lua_callback *lc = wl_container_of(listener, lc, listener);
    struct kiwmi_server *server   = lc->server;
    lua_State *L                  = server->lua->L;
    struct kiwmi_view *view       = data;

    lua_rawgeti(L, LUA_REGISTRYINDEX, lc->callback_ref);

    lua_pushcfunction(L, luaK_kiwmi_view_new);
    lua_pushlightuserdata(L, view);
    if (lua_pcall(L, 1, 1, 0)) {
        wlr_log(WLR_ERROR, "%s", lua_tostring(L, -1));
    }

    if (lua_pcall(L, 1, 0, 0)) {
        wlr_log(WLR_ERROR, "%s", lua_tostring(L, -1));
    }
}

static int
l_kiwmi_server_on_view(lua_State *L)
{
    struct kiwmi_server *server =
        *(struct kiwmi_server **)luaL_checkudata(L, 1, "kiwmi_server");
    luaL_checktype(L, 2, LUA_TFUNCTION);

    lua_pushcfunction(L, luaK_kiwmi_lua_callback_new);
    lua_pushlightuserdata(L, server);
    lua_pushvalue(L, 2);
    lua_pushlightuserdata(L, kiwmi_server_on_view_notify);
    lua_pushlightuserdata(L, &server->desktop.events.view_map);

    if (lua_pcall(L, 4, 1, 0)) {
        wlr_log(WLR_ERROR, "%s", lua_tostring(L, -1));
        return 0;
    }

    return 1;
}

static const luaL_Reg kiwmi_server_events[] = {
    {"view", l_kiwmi_server_on_view},
    {NULL, NULL},
};

int
luaK_kiwmi_server_new(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TLIGHTUSERDATA); // kiwmi_server

    struct kiwmi_server *server = lua_touserdata(L, 1);

    struct kiwmi_server **server_ud = lua_newuserdata(L, sizeof(*server_ud));
    luaL_getmetatable(L, "kiwmi_server");
    lua_setmetatable(L, -2);

    *server_ud = server;

    return 1;
}

int
luaK_kiwmi_server_register(lua_State *L)
{
    luaL_newmetatable(L, "kiwmi_server");

    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, kiwmi_server_methods, 0);

    luaL_newlib(L, kiwmi_server_events);
    lua_setfield(L, -2, "__events");

    return 0;
}
