/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "luak.h"

#include <lauxlib.h>
#include <lualib.h>

bool
luaK_init(struct kiwmi_server *server)
{
    lua_State *L = luaL_newstate();
    if (!L) {
        return false;
    }

    luaL_openlibs(L);

    // TODO: kiwmi library

    server->L = L;
    return true;
}
