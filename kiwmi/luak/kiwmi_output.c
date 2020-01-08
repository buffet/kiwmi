/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "luak/kiwmi_output.h"

#include <lauxlib.h>
#include <wlr/types/wlr_box.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/util/log.h>

#include "desktop/output.h"
#include "luak/kiwmi_lua_callback.h"
#include "server.h"

static int
l_kiwmi_output_auto(lua_State *L)
{
    struct kiwmi_output *output =
        *(struct kiwmi_output **)luaL_checkudata(L, 1, "kiwmi_output");

    struct wlr_output_layout *output_layout = output->desktop->output_layout;

    wlr_output_layout_add_auto(output_layout, output->wlr_output);

    return 0;
}

static int
l_kiwmi_output_move(lua_State *L)
{
    struct kiwmi_output *output =
        *(struct kiwmi_output **)luaL_checkudata(L, 1, "kiwmi_output");
    luaL_checktype(L, 2, LUA_TNUMBER); // x
    luaL_checktype(L, 3, LUA_TNUMBER); // y

    struct wlr_output_layout *output_layout = output->desktop->output_layout;

    int lx = lua_tointeger(L, 2);
    int ly = lua_tointeger(L, 3);

    wlr_output_layout_move(output_layout, output->wlr_output, lx, ly);

    return 0;
}

static int
l_kiwmi_output_name(lua_State *L)
{
    struct kiwmi_output *output =
        *(struct kiwmi_output **)luaL_checkudata(L, 1, "kiwmi_output");

    lua_pushstring(L, output->wlr_output->name);

    return 1;
}

static int
l_kiwmi_output_pos(lua_State *L)
{
    struct kiwmi_output *output =
        *(struct kiwmi_output **)luaL_checkudata(L, 1, "kiwmi_output");

    struct wlr_output_layout *output_layout = output->desktop->output_layout;

    struct wlr_box *box =
        wlr_output_layout_get_box(output_layout, output->wlr_output);

    lua_pushinteger(L, box->x);
    lua_pushinteger(L, box->y);

    return 2;
}

static int
l_kiwmi_output_size(lua_State *L)
{
    struct kiwmi_output *output =
        *(struct kiwmi_output **)luaL_checkudata(L, 1, "kiwmi_output");

    lua_pushinteger(L, output->wlr_output->width);
    lua_pushinteger(L, output->wlr_output->height);

    return 2;
}

static const luaL_Reg kiwmi_output_methods[] = {
    {"auto", l_kiwmi_output_auto},
    {"move", l_kiwmi_output_move},
    {"name", l_kiwmi_output_name},
    {"pos", l_kiwmi_output_pos},
    {"size", l_kiwmi_output_size},
    {NULL, NULL},
};

static void
kiwmi_output_on_destroy_notify(struct wl_listener *listener, void *data)
{
    struct kiwmi_lua_callback *lc = wl_container_of(listener, lc, listener);
    struct kiwmi_server *server   = lc->server;
    lua_State *L                  = server->lua->L;
    struct kiwmi_output *output   = data;

    lua_rawgeti(L, LUA_REGISTRYINDEX, lc->callback_ref);

    lua_pushcfunction(L, luaK_kiwmi_output_new);
    lua_pushlightuserdata(L, output);

    if (lua_pcall(L, 1, 1, 0)) {
        wlr_log(WLR_ERROR, "%s", lua_tostring(L, -1));
        lua_pop(L, 1);
        return;
    }

    if (lua_pcall(L, 1, 0, 0)) {
        wlr_log(WLR_ERROR, "%s", lua_tostring(L, -1));
        lua_pop(L, 1);
    }
}

static int
l_kiwmi_output_on_destroy(lua_State *L)
{
    struct kiwmi_output *output =
        *(struct kiwmi_output **)luaL_checkudata(L, 1, "kiwmi_output");
    luaL_checktype(L, 2, LUA_TFUNCTION);

    struct kiwmi_desktop *desktop = output->desktop;
    struct kiwmi_server *server   = wl_container_of(desktop, server, desktop);

    lua_pushcfunction(L, luaK_kiwmi_lua_callback_new);
    lua_pushlightuserdata(L, server);
    lua_pushvalue(L, 2);
    lua_pushlightuserdata(L, kiwmi_output_on_destroy_notify);
    lua_pushlightuserdata(L, &output->events.destroy);

    if (lua_pcall(L, 4, 1, 0)) {
        wlr_log(WLR_ERROR, "%s", lua_tostring(L, -1));
        return 0;
    }

    return 1;
}

static const luaL_Reg kiwmi_output_events[] = {
    {"destroy", l_kiwmi_output_on_destroy},
    {NULL, NULL},
};

int
luaK_kiwmi_output_new(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TLIGHTUSERDATA); // kiwmi_output

    struct kiwmi_output *output = lua_touserdata(L, 1);

    struct kiwmi_output **output_ud = lua_newuserdata(L, sizeof(*output_ud));
    luaL_getmetatable(L, "kiwmi_output");
    lua_setmetatable(L, -2);

    *output_ud = output;

    return 1;
}

int
luaK_kiwmi_output_register(lua_State *L)
{
    luaL_newmetatable(L, "kiwmi_output");

    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, kiwmi_output_methods, 0);

    luaL_newlib(L, kiwmi_output_events);
    lua_setfield(L, -2, "__events");

    lua_pushcfunction(L, luaK_usertype_ref_equal);
    lua_setfield(L, -2, "__eq");

    return 0;
}
