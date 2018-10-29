/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <poll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "common.h"

int
main(int argc, char *argv[])
{
	argv0 = argv[0];

	if (argc < 2) {
		die("not enough arguments\n");
	}

	int sock_fd;

	if ((sock_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		die("failed to open socket\n");
	}

	struct sockaddr_un sock_addr;
	sock_addr.sun_family = AF_UNIX;

	char *sock_path = getenv(SOCK_ENV_VAR);

	if (sock_path) {
		strncpy(sock_addr.sun_path, sock_path, sizeof(sock_addr.sun_path) - 1);
	} else {
		strncpy(sock_addr.sun_path, SOCK_DEF_PATH, sizeof(sock_addr.sun_path) - 1);
	}

	if (connect(sock_fd, (struct sockaddr *)&sock_addr, sizeof(sock_addr)) < 0) {
		die("failed to connect to socket\n");
	}

	char msg[BUFSIZ];
	size_t msg_len = 0;

	for (int i = 1; i < argc; ++i) {
		msg_len += (size_t)snprintf(msg + msg_len, sizeof(msg), "%s ", argv[i]);
	}

	msg_len -= 1; // remove trailing space

	if (write(sock_fd, msg, msg_len) < 0) {
		die("failed to send message\n");
	}

	struct pollfd  fds[] = {
		{ sock_fd, POLLIN, 0 },
		{ STDOUT_FILENO, POLLHUP, 0 },
	};

	while (poll(fds, 2, -1) > 0) {
		if (fds[1].revents & (POLLERR | POLLHUP)) {
			break;
		}

		if (fds[0].revents & POLLIN) {
			if ((msg_len = read(sock_fd, msg, sizeof(msg) - 1)) > 0) {
				msg[msg_len] = '\0';
				if (msg_len == 1) {
					break;
				}

				printf("%s", msg);
				fflush(stdout);
			} else {
				break;
			}
		}
	}

	close(sock_fd);
}
