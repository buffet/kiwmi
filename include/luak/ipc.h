/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef KIWMI_LUAK_IPC_H
#define KIWMI_LUAK_IPC_H

#include <stdbool.h>

#include "luak/luak.h"
#include "server.h"

bool luaK_ipc_init(struct kiwmi_server *server, struct kiwmi_lua *lua);

#endif /* KIWMI_LUAK_IPC_H */
