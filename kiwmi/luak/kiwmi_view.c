/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "luak/kiwmi_view.h"

#include <string.h>

#include <lauxlib.h>
#include <wlr/util/edges.h>
#include <wlr/util/log.h>

#include "desktop/view.h"
#include "luak/kiwmi_lua_callback.h"
#include "server.h"

static int
l_kiwmi_view_close(lua_State *L)
{
    struct kiwmi_view *view =
        *(struct kiwmi_view **)luaL_checkudata(L, 1, "kiwmi_view");

    view_close(view);

    return 0;
}

static int
l_kiwmi_view_focus(lua_State *L)
{
    struct kiwmi_view *view =
        *(struct kiwmi_view **)luaL_checkudata(L, 1, "kiwmi_view");

    view_focus(view);

    return 0;
}

static int
l_kiwmi_view_hidden(lua_State *L)
{
    struct kiwmi_view *view =
        *(struct kiwmi_view **)luaL_checkudata(L, 1, "kiwmi_view");

    lua_pushboolean(L, view->hidden);

    return 1;
}

static int
l_kiwmi_view_hide(lua_State *L)
{
    struct kiwmi_view *view =
        *(struct kiwmi_view **)luaL_checkudata(L, 1, "kiwmi_view");

    view->hidden = true;

    return 0;
}

static int
l_kiwmi_view_move(lua_State *L)
{
    struct kiwmi_view *view =
        *(struct kiwmi_view **)luaL_checkudata(L, 1, "kiwmi_view");

    luaL_checktype(L, 2, LUA_TNUMBER);
    luaL_checktype(L, 3, LUA_TNUMBER);

    view->x = lua_tonumber(L, 2);
    view->y = lua_tonumber(L, 3);

    return 0;
}

static int
l_kiwmi_view_pos(lua_State *L)
{
    struct kiwmi_view *view =
        *(struct kiwmi_view **)luaL_checkudata(L, 1, "kiwmi_view");

    lua_pushnumber(L, view->x);
    lua_pushnumber(L, view->y);

    return 2;
}

static int
l_kiwmi_view_resize(lua_State *L)
{
    struct kiwmi_view *view =
        *(struct kiwmi_view **)luaL_checkudata(L, 1, "kiwmi_view");
    luaL_checktype(L, 2, LUA_TNUMBER); // w
    luaL_checktype(L, 3, LUA_TNUMBER); // h

    double w = lua_tonumber(L, 2);
    double h = lua_tonumber(L, 3);

    view_set_size(view, w, h);

    return 0;
}

static int
l_kiwmi_view_show(lua_State *L)
{
    struct kiwmi_view *view =
        *(struct kiwmi_view **)luaL_checkudata(L, 1, "kiwmi_view");

    view->hidden = false;

    return 0;
}

static int
l_kiwmi_view_size(lua_State *L)
{
    struct kiwmi_view *view =
        *(struct kiwmi_view **)luaL_checkudata(L, 1, "kiwmi_view");

    uint32_t width;
    uint32_t height;
    view_get_size(view, &width, &height);

    lua_pushinteger(L, width);
    lua_pushinteger(L, height);

    return 2;
}

static int
l_kiwmi_view_tiled(lua_State *L)
{
    struct kiwmi_view *view =
        *(struct kiwmi_view **)luaL_checkudata(L, 1, "kiwmi_view");

    if (lua_isboolean(L, 2)) {
        enum wlr_edges edges = WLR_EDGE_NONE;

        if (lua_toboolean(L, 2)) {
            edges =
                WLR_EDGE_TOP | WLR_EDGE_BOTTOM | WLR_EDGE_LEFT | WLR_EDGE_RIGHT;
        }

        view_set_tiled(view, edges);

        return 0;
    }

    if (lua_istable(L, 2)) {
        enum wlr_edges edges = WLR_EDGE_NONE;

        lua_pushnil(L);
        while (lua_next(L, 2)) {
            if (!lua_isstring(L, -1)) {
                lua_pop(L, 1);
                continue;
            }

            const char *edge = lua_tostring(L, -1);

            switch (edge[0]) {
            case 't':
                edges |= WLR_EDGE_TOP;
                break;
            case 'b':
                edges |= WLR_EDGE_BOTTOM;
                break;
            case 'l':
                edges |= WLR_EDGE_LEFT;
                break;
            case 'r':
                edges |= WLR_EDGE_RIGHT;
                break;
            }

            lua_pop(L, 1);
        }

        view_set_tiled(view, edges);

        return 0;
    }

    return luaL_argerror(L, 2, "expected bool or table");
}

static const luaL_Reg kiwmi_view_methods[] = {
    {"close", l_kiwmi_view_close},
    {"focus", l_kiwmi_view_focus},
    {"hidden", l_kiwmi_view_hidden},
    {"hide", l_kiwmi_view_hide},
    {"move", l_kiwmi_view_move},
    {"on", luaK_callback_register_dispatch},
    {"pos", l_kiwmi_view_pos},
    {"resize", l_kiwmi_view_resize},
    {"show", l_kiwmi_view_show},
    {"size", l_kiwmi_view_size},
    {"tiled", l_kiwmi_view_tiled},
    {NULL, NULL},
};

static void
kiwmi_view_on_destroy_notify(struct wl_listener *listener, void *data)
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
        lua_pop(L, 1);
        return;
    }

    if (lua_pcall(L, 1, 0, 0)) {
        wlr_log(WLR_ERROR, "%s", lua_tostring(L, -1));
        lua_pop(L, 1);
    }
}

static int
l_kiwmi_view_on_destroy(lua_State *L)
{
    struct kiwmi_view *view =
        *(struct kiwmi_view **)luaL_checkudata(L, 1, "kiwmi_view");
    luaL_checktype(L, 2, LUA_TFUNCTION);

    struct kiwmi_desktop *desktop = view->desktop;
    struct kiwmi_server *server   = wl_container_of(desktop, server, desktop);

    lua_pushcfunction(L, luaK_kiwmi_lua_callback_new);
    lua_pushlightuserdata(L, server);
    lua_pushvalue(L, 2);
    lua_pushlightuserdata(L, kiwmi_view_on_destroy_notify);
    lua_pushlightuserdata(L, &view->events.unmap);

    if (lua_pcall(L, 4, 1, 0)) {
        wlr_log(WLR_ERROR, "%s", lua_tostring(L, -1));
        return 0;
    }

    return 1;
}

static const luaL_Reg kiwmi_view_events[] = {
    {"destroy", l_kiwmi_view_on_destroy},
    {NULL, NULL},
};

int
luaK_kiwmi_view_new(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TLIGHTUSERDATA); // kiwmi_view

    struct kiwmi_view *view = lua_touserdata(L, 1);

    struct kiwmi_view **view_ud = lua_newuserdata(L, sizeof(*view_ud));
    luaL_getmetatable(L, "kiwmi_view");
    lua_setmetatable(L, -2);

    *view_ud = view;

    return 1;
}

int
luaK_kiwmi_view_register(lua_State *L)
{
    luaL_newmetatable(L, "kiwmi_view");

    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, kiwmi_view_methods, 0);

    luaL_newlib(L, kiwmi_view_events);
    lua_setfield(L, -2, "__events");

    lua_pushcfunction(L, luaK_usertype_ref_equal);
    lua_setfield(L, -2, "__eq");

    return 0;
}
