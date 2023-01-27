/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "luak/kiwmi_output.h"

#include <lauxlib.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/util/box.h>
#include <wlr/util/log.h>

#include "desktop/output.h"
#include "luak/kiwmi_lua_callback.h"
#include "luak/lua_compat.h"
#include "server.h"

static int
l_kiwmi_output_auto(lua_State *L)
{
    struct kiwmi_object *obj =
        *(struct kiwmi_object **)luaL_checkudata(L, 1, "kiwmi_output");

    if (!obj->valid) {
        return luaL_error(L, "kiwmi_output no longer valid");
    }

    struct kiwmi_output *output             = obj->object;
    struct wlr_output_layout *output_layout = output->desktop->output_layout;

    wlr_output_layout_add_auto(output_layout, output->wlr_output);

    return 0;
}

static int
l_kiwmi_output_move(lua_State *L)
{
    struct kiwmi_object *obj =
        *(struct kiwmi_object **)luaL_checkudata(L, 1, "kiwmi_output");
    luaL_checktype(L, 2, LUA_TNUMBER); // x
    luaL_checktype(L, 3, LUA_TNUMBER); // y

    if (!obj->valid) {
        return luaL_error(L, "kiwmi_output no longer valid");
    }

    struct kiwmi_output *output             = obj->object;
    struct wlr_output_layout *output_layout = output->desktop->output_layout;

    int lx = lua_tonumber(L, 2);
    int ly = lua_tonumber(L, 3);

    wlr_output_layout_move(output_layout, output->wlr_output, lx, ly);

    return 0;
}

static int
l_kiwmi_output_name(lua_State *L)
{
    struct kiwmi_object *obj =
        *(struct kiwmi_object **)luaL_checkudata(L, 1, "kiwmi_output");

    if (!obj->valid) {
        return luaL_error(L, "kiwmi_output no longer valid");
    }

    struct kiwmi_output *output = obj->object;

    lua_pushstring(L, output->wlr_output->name);

    return 1;
}

static int
l_kiwmi_output_pos(lua_State *L)
{
    struct kiwmi_object *obj =
        *(struct kiwmi_object **)luaL_checkudata(L, 1, "kiwmi_output");

    if (!obj->valid) {
        return luaL_error(L, "kiwmi_output no longer valid");
    }

    struct kiwmi_output *output             = obj->object;
    struct wlr_output_layout *output_layout = output->desktop->output_layout;

    struct wlr_box box;
    wlr_output_layout_get_box(output_layout, output->wlr_output, &box);

    lua_pushinteger(L, box.x);
    lua_pushinteger(L, box.y);

    return 2;
}

static int
l_kiwmi_output_size(lua_State *L)
{
    struct kiwmi_object *obj =
        *(struct kiwmi_object **)luaL_checkudata(L, 1, "kiwmi_output");

    if (!obj->valid) {
        return luaL_error(L, "kiwmi_output no longer valid");
    }

    struct kiwmi_output *output = obj->object;

    lua_pushinteger(L, output->wlr_output->width);
    lua_pushinteger(L, output->wlr_output->height);

    return 2;
}

static int
l_kiwmi_output_usable_area(lua_State *L)
{
    struct kiwmi_object *obj =
        *(struct kiwmi_object **)luaL_checkudata(L, 1, "kiwmi_output");

    if (!obj->valid) {
        return luaL_error(L, "kiwmi_output no longer valid");
    }

    struct kiwmi_output *output = obj->object;

    lua_newtable(L);

    lua_pushinteger(L, output->usable_area.x);
    lua_setfield(L, -2, "x");

    lua_pushinteger(L, output->usable_area.y);
    lua_setfield(L, -2, "y");

    lua_pushinteger(L, output->usable_area.width);
    lua_setfield(L, -2, "width");

    lua_pushinteger(L, output->usable_area.height);
    lua_setfield(L, -2, "height");

    return 1;
}

static const luaL_Reg kiwmi_output_methods[] = {
    {"auto", l_kiwmi_output_auto},
    {"move", l_kiwmi_output_move},
    {"name", l_kiwmi_output_name},
    {"on", luaK_callback_register_dispatch},
    {"pos", l_kiwmi_output_pos},
    {"size", l_kiwmi_output_size},
    {"usable_area", l_kiwmi_output_usable_area},
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
    lua_pushlightuserdata(L, server->lua);
    lua_pushlightuserdata(L, output);

    if (lua_pcall(L, 2, 1, 0)) {
        wlr_log(WLR_ERROR, "%s", lua_tostring(L, -1));
        lua_pop(L, 1);
        return;
    }

    if (lua_pcall(L, 1, 0, 0)) {
        wlr_log(WLR_ERROR, "%s", lua_tostring(L, -1));
        lua_pop(L, 1);
    }
}

static void
kiwmi_output_on_resize_notify(struct wl_listener *listener, void *data)
{
    struct kiwmi_lua_callback *lc = wl_container_of(listener, lc, listener);
    struct kiwmi_server *server   = lc->server;
    lua_State *L                  = server->lua->L;
    struct kiwmi_output *output   = data;

    int width;
    int height;
    wlr_output_transformed_resolution(output->wlr_output, &width, &height);

    lua_rawgeti(L, LUA_REGISTRYINDEX, lc->callback_ref);

    lua_newtable(L);

    lua_pushcfunction(L, luaK_kiwmi_output_new);
    lua_pushlightuserdata(L, server->lua);
    lua_pushlightuserdata(L, output);

    if (lua_pcall(L, 2, 1, 0)) {
        wlr_log(WLR_ERROR, "%s", lua_tostring(L, -1));
        lua_pop(L, 1);
        return;
    }

    lua_setfield(L, -2, "output");

    lua_pushinteger(L, width);
    lua_setfield(L, -2, "width");

    lua_pushinteger(L, height);
    lua_setfield(L, -2, "height");

    if (lua_pcall(L, 1, 0, 0)) {
        wlr_log(WLR_ERROR, "%s", lua_tostring(L, -1));
        lua_pop(L, 1);
    }
}

static void
kiwmi_output_on_usable_area_change_notify(
    struct wl_listener *listener,
    void *data)
{
    struct kiwmi_lua_callback *lc = wl_container_of(listener, lc, listener);
    struct kiwmi_server *server   = lc->server;
    lua_State *L                  = server->lua->L;
    struct kiwmi_output *output   = data;

    lua_rawgeti(L, LUA_REGISTRYINDEX, lc->callback_ref);

    lua_newtable(L);

    lua_pushcfunction(L, luaK_kiwmi_output_new);
    lua_pushlightuserdata(L, server->lua);
    lua_pushlightuserdata(L, output);

    if (lua_pcall(L, 2, 1, 0)) {
        wlr_log(WLR_ERROR, "%s", lua_tostring(L, -1));
        lua_pop(L, 1);
        return;
    }

    lua_setfield(L, -2, "output");

    lua_pushinteger(L, output->usable_area.x);
    lua_setfield(L, -2, "x");

    lua_pushinteger(L, output->usable_area.y);
    lua_setfield(L, -2, "y");

    lua_pushinteger(L, output->usable_area.width);
    lua_setfield(L, -2, "width");

    lua_pushinteger(L, output->usable_area.height);
    lua_setfield(L, -2, "height");

    if (lua_pcall(L, 1, 0, 0)) {
        wlr_log(WLR_ERROR, "%s", lua_tostring(L, -1));
        lua_pop(L, 1);
    }
}

static int
l_kiwmi_output_on_destroy(lua_State *L)
{
    struct kiwmi_object *obj =
        *(struct kiwmi_object **)luaL_checkudata(L, 1, "kiwmi_output");
    luaL_checktype(L, 2, LUA_TFUNCTION);

    if (!obj->valid) {
        return luaL_error(L, "kiwmi_output no longer valid");
    }

    struct kiwmi_output *output   = obj->object;
    struct kiwmi_desktop *desktop = output->desktop;
    struct kiwmi_server *server   = wl_container_of(desktop, server, desktop);

    lua_pushcfunction(L, luaK_kiwmi_lua_callback_new);
    lua_pushlightuserdata(L, server);
    lua_pushvalue(L, 2);
    lua_pushlightuserdata(L, kiwmi_output_on_destroy_notify);
    lua_pushlightuserdata(L, &obj->events.destroy);
    lua_pushlightuserdata(L, obj);

    if (lua_pcall(L, 5, 0, 0)) {
        wlr_log(WLR_ERROR, "%s", lua_tostring(L, -1));
        return 0;
    }

    return 0;
}

static int
l_kiwmi_output_on_resize(lua_State *L)
{
    struct kiwmi_object *obj =
        *(struct kiwmi_object **)luaL_checkudata(L, 1, "kiwmi_output");
    luaL_checktype(L, 2, LUA_TFUNCTION);

    if (!obj->valid) {
        return luaL_error(L, "kiwmi_output no longer valid");
    }

    struct kiwmi_output *output   = obj->object;
    struct kiwmi_desktop *desktop = output->desktop;
    struct kiwmi_server *server   = wl_container_of(desktop, server, desktop);

    lua_pushcfunction(L, luaK_kiwmi_lua_callback_new);
    lua_pushlightuserdata(L, server);
    lua_pushvalue(L, 2);
    lua_pushlightuserdata(L, kiwmi_output_on_resize_notify);
    lua_pushlightuserdata(L, &output->events.resize);
    lua_pushlightuserdata(L, obj);

    if (lua_pcall(L, 5, 0, 0)) {
        wlr_log(WLR_ERROR, "%s", lua_tostring(L, -1));
        return 0;
    }

    return 0;
}

static int
l_kiwmi_output_on_usable_area_change(lua_State *L)
{
    struct kiwmi_object *obj =
        *(struct kiwmi_object **)luaL_checkudata(L, 1, "kiwmi_output");
    luaL_checktype(L, 2, LUA_TFUNCTION);

    if (!obj->valid) {
        return luaL_error(L, "kiwmi_output no longer valid");
    }

    struct kiwmi_output *output   = obj->object;
    struct kiwmi_desktop *desktop = output->desktop;
    struct kiwmi_server *server   = wl_container_of(desktop, server, desktop);

    lua_pushcfunction(L, luaK_kiwmi_lua_callback_new);
    lua_pushlightuserdata(L, server);
    lua_pushvalue(L, 2);
    lua_pushlightuserdata(L, kiwmi_output_on_usable_area_change_notify);
    lua_pushlightuserdata(L, &output->events.usable_area_change);
    lua_pushlightuserdata(L, obj);

    if (lua_pcall(L, 5, 0, 0)) {
        wlr_log(WLR_ERROR, "%s", lua_tostring(L, -1));
        return 0;
    }

    return 0;
}

static const luaL_Reg kiwmi_output_events[] = {
    {"destroy", l_kiwmi_output_on_destroy},
    {"resize", l_kiwmi_output_on_resize},
    {"usable_area_change", l_kiwmi_output_on_usable_area_change},
    {NULL, NULL},
};

int
luaK_kiwmi_output_new(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TLIGHTUSERDATA); // kiwmi_lua
    luaL_checktype(L, 2, LUA_TLIGHTUSERDATA); // kiwmi_output

    struct kiwmi_lua *lua       = lua_touserdata(L, 1);
    struct kiwmi_output *output = lua_touserdata(L, 2);

    struct kiwmi_object *obj =
        luaK_get_kiwmi_object(lua, output, &output->events.destroy);

    struct kiwmi_object **output_ud = lua_newuserdata(L, sizeof(*output_ud));
    luaL_getmetatable(L, "kiwmi_output");
    lua_setmetatable(L, -2);

    *output_ud = obj;

    return 1;
}

int
luaK_kiwmi_output_register(lua_State *L)
{
    luaL_newmetatable(L, "kiwmi_output");

    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaC_setfuncs(L, kiwmi_output_methods, 0);

    luaC_newlib(L, kiwmi_output_events);
    lua_setfield(L, -2, "__events");

    lua_pushcfunction(L, luaK_usertype_ref_equal);
    lua_setfield(L, -2, "__eq");

    lua_pushcfunction(L, luaK_kiwmi_object_gc);
    lua_setfield(L, -2, "__gc");

    return 0;
}
