/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef KIWMI_LUAK_KIWMI_SERVER_H
#define KIWMI_LUAK_KIWMI_SERVER_H

#include <lua.h>

int luaK_kiwmi_server_new(lua_State *L);
int luaK_kiwmi_server_register(lua_State *L);

#endif /* KIWMI_LUAK_KIWMI_SERVER_H */
