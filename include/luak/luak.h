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

#include "server.h"

struct kiwmi_lua {
    lua_State *L;
    struct wl_list callbacks; // lua_callback::link
};

int luaK_callback_register_dispatch(lua_State *L);
struct kiwmi_lua *luaK_create(struct kiwmi_server *server);
bool luaK_dofile(struct kiwmi_lua *lua, const char *config_path);
void luaK_destroy(struct kiwmi_lua *lua);

#endif /* KIWMI_LUAK_LUAK_H */
