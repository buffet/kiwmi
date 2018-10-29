/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "ipc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "common.h"

int g_sock_fd;

void
init_socket(void)
{
	struct sockaddr_un sock_addr;

	memset(&sock_addr, 0, sizeof(sock_addr));

	char *sock_path = getenv(SOCK_ENV_VAR);

	if (sock_path) {
		strncpy(sock_addr.sun_path, sock_path, sizeof(sock_addr.sun_path) - 1);
	} else {
		strncpy(sock_addr.sun_path, SOCK_DEF_PATH, sizeof(sock_addr.sun_path) - 1);
	}

	sock_addr.sun_family = AF_UNIX;

	if ((g_sock_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		die("failed to create socket\n");
	}

	unlink(sock_addr.sun_path);

	if (bind(g_sock_fd, (struct sockaddr *)&sock_addr, sizeof(sock_addr)) < 0) {
		die("failed to bind socket\n");
	}

	if (listen(g_sock_fd, 1) < 0) {
		die("failed to listen to socket\n");
	}
}
