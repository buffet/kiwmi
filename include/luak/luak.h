/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef KIWMI_LUAK_LUAK_H
#define KIWMI_LUAK_LUAK_H

#include <stdbool.h>

#include <lua.h>
#include <wayland-server.h>

#include "server.h"

struct kiwmi_lua {
    lua_State *L;
    int objects;
    struct wl_list scheduled_callbacks;
    struct wl_global *global;
};

struct kiwmi_object {
    struct kiwmi_lua *lua;

    void *object;
    size_t refcount;
    bool valid;

    struct wl_listener destroy;
    struct wl_list callbacks; // struct kiwmi_lua_callback::link

    struct {
        struct wl_signal destroy;
    } events;
};

void *luaK_toudata(lua_State *L, int ud, const char *tname);
int luaK_kiwmi_object_gc(lua_State *L);
struct kiwmi_object *luaK_get_kiwmi_object(
    struct kiwmi_lua *lua,
    void *ptr,
    struct wl_signal *destroy);
int luaK_callback_register_dispatch(lua_State *L);
int luaK_usertype_ref_equal(lua_State *L);
struct kiwmi_lua *luaK_create(struct kiwmi_server *server);
bool luaK_dofile(struct kiwmi_lua *lua, const char *config_path);
void luaK_destroy(struct kiwmi_lua *lua);

#endif /* KIWMI_LUAK_LUAK_H */
