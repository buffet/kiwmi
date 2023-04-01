/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "luak/kiwmi_view.h"

#include <string.h>

#include <lauxlib.h>
#include <wayland-server.h>
#include <wlr/types/wlr_xdg_decoration_v1.h>
#include <wlr/util/edges.h>
#include <wlr/util/log.h>

#include "desktop/desktop_surface.h"
#include "desktop/output.h"
#include "desktop/view.h"
#include "desktop/xdg_shell.h"
#include "input/seat.h"
#include "luak/kiwmi_lua_callback.h"
#include "luak/kiwmi_output.h"
#include "server.h"

static int
l_kiwmi_view_app_id(lua_State *L)
{
    struct kiwmi_object *obj =
        *(struct kiwmi_object **)luaL_checkudata(L, 1, "kiwmi_view");

    if (!obj->valid) {
        return luaL_error(L, "kiwmi_view no longer valid");
    }

    struct kiwmi_view *view = obj->object;

    lua_pushstring(L, view_get_app_id(view));

    return 1;
}

static int
l_kiwmi_view_close(lua_State *L)
{
    struct kiwmi_object *obj =
        *(struct kiwmi_object **)luaL_checkudata(L, 1, "kiwmi_view");

    if (!obj->valid) {
        return luaL_error(L, "kiwmi_view no longer valid");
    }

    struct kiwmi_view *view = obj->object;

    view_close(view);

    return 0;
}

static int
l_kiwmi_view_csd(lua_State *L)
{
    struct kiwmi_object *obj =
        *(struct kiwmi_object **)luaL_checkudata(L, 1, "kiwmi_view");
    luaL_checktype(L, 2, LUA_TBOOLEAN);

    if (!obj->valid) {
        return luaL_error(L, "kiwmi_view no longer valid");
    }

    struct kiwmi_view *view = obj->object;

    struct kiwmi_xdg_decoration *decoration = view->decoration;
    if (!decoration) {
        return 0;
    }

    enum wlr_xdg_toplevel_decoration_v1_mode mode;
    if (lua_toboolean(L, 2)) {
        mode = WLR_XDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE;
    } else {
        mode = WLR_XDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE;
    }

    wlr_xdg_toplevel_decoration_v1_set_mode(decoration->wlr_decoration, mode);

    return 0;
}

static int
l_kiwmi_view_focus(lua_State *L)
{
    struct kiwmi_object *obj =
        *(struct kiwmi_object **)luaL_checkudata(L, 1, "kiwmi_view");

    if (!obj->valid) {
        return luaL_error(L, "kiwmi_view no longer valid");
    }

    struct kiwmi_view *view       = obj->object;
    struct kiwmi_desktop *desktop = view->desktop;
    struct kiwmi_server *server   = wl_container_of(desktop, server, desktop);
    struct kiwmi_seat *seat       = server->input.seat;

    seat_focus_view(seat, view);

    return 0;
}

static int
l_kiwmi_view_hidden(lua_State *L)
{
    struct kiwmi_object *obj =
        *(struct kiwmi_object **)luaL_checkudata(L, 1, "kiwmi_view");

    if (!obj->valid) {
        return luaL_error(L, "kiwmi_view no longer valid");
    }

    struct kiwmi_view *view = obj->object;

    int lx, ly; // unused
    bool enabled =
        wlr_scene_node_coords(&view->desktop_surface.tree->node, &lx, &ly);

    lua_pushboolean(L, !enabled);

    return 1;
}

static int
l_kiwmi_view_hide(lua_State *L)
{
    struct kiwmi_object *obj =
        *(struct kiwmi_object **)luaL_checkudata(L, 1, "kiwmi_view");

    if (!obj->valid) {
        return luaL_error(L, "kiwmi_view no longer valid");
    }

    struct kiwmi_view *view = obj->object;

    view_set_hidden(view, true);

    return 0;
}

static int
l_kiwmi_view_id(lua_State *L)
{
    struct kiwmi_object *obj =
        *(struct kiwmi_object **)luaL_checkudata(L, 1, "kiwmi_view");

    if (!obj->valid) {
        return luaL_error(L, "kiwmi_view no longer valid");
    }

    struct kiwmi_view *view = obj->object;

    lua_pushnumber(L, (lua_Number)(size_t)view);

    return 1;
}

static int
l_kiwmi_view_imove(lua_State *L)
{
    struct kiwmi_object *obj =
        *(struct kiwmi_object **)luaL_checkudata(L, 1, "kiwmi_view");

    if (!obj->valid) {
        return luaL_error(L, "kiwmi_view no longer valid");
    }

    struct kiwmi_view *view = obj->object;

    view_move(view);

    return 0;
}

static int
l_kiwmi_view_iresize(lua_State *L)
{
    struct kiwmi_object *obj =
        *(struct kiwmi_object **)luaL_checkudata(L, 1, "kiwmi_view");
    luaL_checktype(L, 2, LUA_TTABLE);

    if (!obj->valid) {
        return luaL_error(L, "kiwmi_view no longer valid");
    }

    struct kiwmi_view *view = obj->object;

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

    view_resize(view, edges);

    return 0;
}

static int
l_kiwmi_view_move(lua_State *L)
{
    struct kiwmi_object *obj =
        *(struct kiwmi_object **)luaL_checkudata(L, 1, "kiwmi_view");

    if (!obj->valid) {
        return luaL_error(L, "kiwmi_view no longer valid");
    }

    struct kiwmi_view *view = obj->object;

    luaL_checktype(L, 2, LUA_TNUMBER);
    luaL_checktype(L, 3, LUA_TNUMBER);

    uint32_t x = lua_tonumber(L, 2);
    uint32_t y = lua_tonumber(L, 3);

    view_set_pos(view, x, y);

    return 0;
}

static int
l_kiwmi_view_pid(lua_State *L)
{
    struct kiwmi_object *obj =
        *(struct kiwmi_object **)luaL_checkudata(L, 1, "kiwmi_view");

    if (!obj->valid) {
        return luaL_error(L, "kiwmi_view no longer valid");
    }

    struct kiwmi_view *view = obj->object;

    lua_pushinteger(L, view_get_pid(view));

    return 1;
}

static int
l_kiwmi_view_pos(lua_State *L)
{
    struct kiwmi_object *obj =
        *(struct kiwmi_object **)luaL_checkudata(L, 1, "kiwmi_view");

    if (!obj->valid) {
        return luaL_error(L, "kiwmi_view no longer valid");
    }

    struct kiwmi_view *view = obj->object;

    int lx, ly;
    desktop_surface_get_pos(&view->desktop_surface, &lx, &ly);

    lua_pushinteger(L, lx);
    lua_pushinteger(L, ly);

    return 2;
}

static int
l_kiwmi_view_resize(lua_State *L)
{
    struct kiwmi_object *obj =
        *(struct kiwmi_object **)luaL_checkudata(L, 1, "kiwmi_view");
    luaL_checktype(L, 2, LUA_TNUMBER); // w
    luaL_checktype(L, 3, LUA_TNUMBER); // h

    if (!obj->valid) {
        return luaL_error(L, "kiwmi_view no longer valid");
    }

    struct kiwmi_view *view = obj->object;

    double w = lua_tonumber(L, 2);
    double h = lua_tonumber(L, 3);

    view_set_size(view, w, h);

    return 0;
}

static int
l_kiwmi_view_show(lua_State *L)
{
    struct kiwmi_object *obj =
        *(struct kiwmi_object **)luaL_checkudata(L, 1, "kiwmi_view");

    if (!obj->valid) {
        return luaL_error(L, "kiwmi_view no longer valid");
    }

    struct kiwmi_view *view = obj->object;

    view_set_hidden(view, false);

    return 0;
}

static int
l_kiwmi_view_size(lua_State *L)
{
    struct kiwmi_object *obj =
        *(struct kiwmi_object **)luaL_checkudata(L, 1, "kiwmi_view");

    if (!obj->valid) {
        return luaL_error(L, "kiwmi_view no longer valid");
    }

    struct kiwmi_view *view = obj->object;

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
    struct kiwmi_object *obj =
        *(struct kiwmi_object **)luaL_checkudata(L, 1, "kiwmi_view");

    if (!obj->valid) {
        return luaL_error(L, "kiwmi_view no longer valid");
    }

    struct kiwmi_view *view = obj->object;

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

static int
l_kiwmi_view_title(lua_State *L)
{
    struct kiwmi_object *obj =
        *(struct kiwmi_object **)luaL_checkudata(L, 1, "kiwmi_view");

    if (!obj->valid) {
        return luaL_error(L, "kiwmi_view no longer valid");
    }

    struct kiwmi_view *view = obj->object;

    lua_pushstring(L, view_get_title(view));

    return 1;
}

static const luaL_Reg kiwmi_view_methods[] = {
    {"app_id", l_kiwmi_view_app_id},
    {"close", l_kiwmi_view_close},
    {"csd", l_kiwmi_view_csd},
    {"focus", l_kiwmi_view_focus},
    {"hidden", l_kiwmi_view_hidden},
    {"hide", l_kiwmi_view_hide},
    {"id", l_kiwmi_view_id},
    {"imove", l_kiwmi_view_imove},
    {"iresize", l_kiwmi_view_iresize},
    {"move", l_kiwmi_view_move},
    {"on", luaK_callback_register_dispatch},
    {"pid", l_kiwmi_view_pid},
    {"pos", l_kiwmi_view_pos},
    {"resize", l_kiwmi_view_resize},
    {"show", l_kiwmi_view_show},
    {"size", l_kiwmi_view_size},
    {"tiled", l_kiwmi_view_tiled},
    {"title", l_kiwmi_view_title},
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
    lua_pushlightuserdata(L, server->lua);
    lua_pushlightuserdata(L, view);

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
kiwmi_view_on_request_move_notify(struct wl_listener *listener, void *data)
{
    struct kiwmi_lua_callback *lc = wl_container_of(listener, lc, listener);
    struct kiwmi_server *server   = lc->server;
    lua_State *L                  = server->lua->L;
    struct kiwmi_view *view       = data;

    lua_rawgeti(L, LUA_REGISTRYINDEX, lc->callback_ref);

    lua_pushcfunction(L, luaK_kiwmi_view_new);
    lua_pushlightuserdata(L, server->lua);
    lua_pushlightuserdata(L, view);

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
kiwmi_view_on_request_resize_notify(struct wl_listener *listener, void *data)
{
    struct kiwmi_lua_callback *lc = wl_container_of(listener, lc, listener);
    struct kiwmi_server *server   = lc->server;
    lua_State *L                  = server->lua->L;

    struct kiwmi_request_resize_event *event = data;
    struct kiwmi_view *view                  = event->view;

    lua_rawgeti(L, LUA_REGISTRYINDEX, lc->callback_ref);

    lua_newtable(L);

    lua_pushcfunction(L, luaK_kiwmi_view_new);
    lua_pushlightuserdata(L, server->lua);
    lua_pushlightuserdata(L, view);

    if (lua_pcall(L, 2, 1, 0)) {
        wlr_log(WLR_ERROR, "%s", lua_tostring(L, -1));
        lua_pop(L, 1);
        return;
    }

    lua_setfield(L, -2, "view");

    lua_newtable(L);
    int idx = 1;

    if (event->edges & WLR_EDGE_TOP) {
        lua_pushstring(L, "top");
        lua_rawseti(L, -2, idx++);
    }

    if (event->edges & WLR_EDGE_BOTTOM) {
        lua_pushstring(L, "bottom");
        lua_rawseti(L, -2, idx++);
    }

    if (event->edges & WLR_EDGE_LEFT) {
        lua_pushstring(L, "left");
        lua_rawseti(L, -2, idx++);
    }

    if (event->edges & WLR_EDGE_RIGHT) {
        lua_pushstring(L, "right");
        lua_rawseti(L, -2, idx++);
    }

    lua_setfield(L, -2, "edges");

    if (lua_pcall(L, 1, 0, 0)) {
        wlr_log(WLR_ERROR, "%s", lua_tostring(L, -1));
        lua_pop(L, 1);
    }
}

static int
l_kiwmi_view_on_destroy(lua_State *L)
{
    struct kiwmi_object *obj =
        *(struct kiwmi_object **)luaL_checkudata(L, 1, "kiwmi_view");
    luaL_checktype(L, 2, LUA_TFUNCTION);

    if (!obj->valid) {
        return luaL_error(L, "kiwmi_view no longer valid");
    }

    struct kiwmi_view *view       = obj->object;
    struct kiwmi_desktop *desktop = view->desktop;
    struct kiwmi_server *server   = wl_container_of(desktop, server, desktop);

    lua_pushcfunction(L, luaK_kiwmi_lua_callback_new);
    lua_pushlightuserdata(L, server);
    lua_pushvalue(L, 2);
    lua_pushlightuserdata(L, kiwmi_view_on_destroy_notify);
    lua_pushlightuserdata(L, &obj->events.destroy);
    lua_pushlightuserdata(L, obj);

    if (lua_pcall(L, 5, 0, 0)) {
        wlr_log(WLR_ERROR, "%s", lua_tostring(L, -1));
        return 0;
    }

    return 0;
}

static int
l_kiwmi_view_on_post_render(lua_State *UNUSED(L))
{
    // noop
    return 0;
}

static int
l_kiwmi_view_on_pre_render(lua_State *UNUSED(L))
{
    // noop
    return 0;
}

static int
l_kiwmi_view_on_request_move(lua_State *L)
{
    struct kiwmi_object *obj =
        *(struct kiwmi_object **)luaL_checkudata(L, 1, "kiwmi_view");
    luaL_checktype(L, 2, LUA_TFUNCTION);

    if (!obj->valid) {
        return luaL_error(L, "kiwmi_view no longer valid");
    }

    struct kiwmi_view *view       = obj->object;
    struct kiwmi_desktop *desktop = view->desktop;
    struct kiwmi_server *server   = wl_container_of(desktop, server, desktop);

    lua_pushcfunction(L, luaK_kiwmi_lua_callback_new);
    lua_pushlightuserdata(L, server);
    lua_pushvalue(L, 2);
    lua_pushlightuserdata(L, kiwmi_view_on_request_move_notify);
    lua_pushlightuserdata(L, &view->events.request_move);
    lua_pushlightuserdata(L, obj);

    if (lua_pcall(L, 5, 0, 0)) {
        wlr_log(WLR_ERROR, "%s", lua_tostring(L, -1));
        return 0;
    }

    return 0;
}

static int
l_kiwmi_view_on_request_resize(lua_State *L)
{
    struct kiwmi_object *obj =
        *(struct kiwmi_object **)luaL_checkudata(L, 1, "kiwmi_view");
    luaL_checktype(L, 2, LUA_TFUNCTION);

    if (!obj->valid) {
        return luaL_error(L, "kiwmi_view no longer valid");
    }

    struct kiwmi_view *view       = obj->object;
    struct kiwmi_desktop *desktop = view->desktop;
    struct kiwmi_server *server   = wl_container_of(desktop, server, desktop);

    lua_pushcfunction(L, luaK_kiwmi_lua_callback_new);
    lua_pushlightuserdata(L, server);
    lua_pushvalue(L, 2);
    lua_pushlightuserdata(L, kiwmi_view_on_request_resize_notify);
    lua_pushlightuserdata(L, &view->events.request_resize);
    lua_pushlightuserdata(L, obj);

    if (lua_pcall(L, 5, 0, 0)) {
        wlr_log(WLR_ERROR, "%s", lua_tostring(L, -1));
        return 0;
    }

    return 0;
}

static const luaL_Reg kiwmi_view_events[] = {
    {"destroy", l_kiwmi_view_on_destroy},
    {"post_render", l_kiwmi_view_on_post_render},
    {"pre_render", l_kiwmi_view_on_pre_render},
    {"request_move", l_kiwmi_view_on_request_move},
    {"request_resize", l_kiwmi_view_on_request_resize},
    {NULL, NULL},
};

int
luaK_kiwmi_view_new(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TLIGHTUSERDATA); // kiwmi_lua
    luaL_checktype(L, 2, LUA_TLIGHTUSERDATA); // kiwmi_view

    struct kiwmi_lua *lua   = lua_touserdata(L, 1);
    struct kiwmi_view *view = lua_touserdata(L, 2);

    struct kiwmi_object *obj =
        luaK_get_kiwmi_object(lua, view, &view->events.unmap);

    struct kiwmi_object **view_ud = lua_newuserdata(L, sizeof(*view_ud));
    luaL_getmetatable(L, "kiwmi_view");
    lua_setmetatable(L, -2);

    *view_ud = obj;

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

    lua_pushcfunction(L, luaK_kiwmi_object_gc);
    lua_setfield(L, -2, "__gc");

    return 0;
}
