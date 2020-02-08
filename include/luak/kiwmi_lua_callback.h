/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef KIWMI_LUAK_KIWMI_LUA_CALLBACK_H
#define KIWMI_LUAK_KIWMI_LUA_CALLBACK_H

#include <lua.h>

#include "luak/luak.h"

struct kiwmi_lua_callback {
    struct wl_list link;
    struct kiwmi_server *server;
    int callback_ref;
    union {
        struct wl_event_source *event_source;
        struct wl_listener listener;
    };
};

int luaK_kiwmi_lua_callback_new(lua_State *L);

#endif /* KIWMI_LUAK_KIWMI_LUA_CALLBACK_H */
