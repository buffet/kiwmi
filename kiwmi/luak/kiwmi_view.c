/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "luak/kiwmi_view.h"

#include <lauxlib.h>

#include "desktop/view.h"

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
l_kiwmi_view_show(lua_State *L)
{
    struct kiwmi_view *view =
        *(struct kiwmi_view **)luaL_checkudata(L, 1, "kiwmi_view");

    view->hidden = false;

    return 0;
}

static const luaL_Reg kiwmi_view_methods[] = {
    {"close", l_kiwmi_view_close},
    {"focus", l_kiwmi_view_focus},
    {"hidden", l_kiwmi_view_hidden},
    {"hide", l_kiwmi_view_hide},
    {"move", l_kiwmi_view_move},
    {"show", l_kiwmi_view_show},
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

    return 0;
}
