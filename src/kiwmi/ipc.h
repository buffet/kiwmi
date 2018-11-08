/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef IPC_H
#define IPC_H

#include <stdio.h>

void init_socket(void);
void handle_ipc_event(FILE *client, char *msg);

extern int g_sock_fd;

#endif /* IPC_H */
