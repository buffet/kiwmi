/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef KIWMI_LUAK_LUA_COMPAT_H
#define KIWMI_LUAK_LUA_COMPAT_H

#include <lauxlib.h>
#include <lua.h>

#define luaC_newlibtable(L, l)                                                 \
    lua_createtable(L, 0, sizeof(l) / sizeof((l)[0]) - 1)

#define luaC_newlib(L, l) (luaC_newlibtable(L, l), luaC_setfuncs(L, l, 0))

void luaC_setfuncs(lua_State *L, const luaL_Reg *l, int nup);

#endif /* KIWMI_LUAK_LUA_COMPAT_H */
