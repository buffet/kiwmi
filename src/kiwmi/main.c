/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <getopt.h>
#include <limits.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#include <xcb/xcb.h>

#include "ipc.h"
#include "xcb.h"

#include "common.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))

static void exec_config(const char *path);
static void sig_handler(int sig);

int g_is_about_to_quit = 0;

int
main(int argc, char *argv[])
{
	argv0 = argv[0];

	char config_path[PATH_MAX];
	int option;

	config_path[0] = '\0';

	while ((option = getopt(argc, argv, "hvc:")) != -1) {
		switch (option) {
			case 'h':
				printf("Usage: %s [-h|-v|-c <config_path>]\n", argv0);
				exit(EXIT_SUCCESS);
			case 'v':
				printf("v" VERSION_STRING "\n");
				exit(EXIT_SUCCESS);
			case 'c':
				strncpy(config_path, optarg, sizeof(config_path));
				break;
		}
	}

	// default config path
	if (!config_path[0]) {
		char *config_home = getenv("XDG_CONFIG_HOME");
		if (config_home) {
			snprintf(
				config_path,
				sizeof(config_path),
				"%s/%s",
				config_home,
				CONFIG_FILE
			);
		} else {
			// ${HOME}/.config as fallback
			snprintf(
				config_path,
				sizeof(config_path),
				"%s/%s",
				getenv("HOME"),
				".config/" CONFIG_FILE
			);
		}
	}

	signal(SIGINT, sig_handler);
	signal(SIGHUP, sig_handler);
	signal(SIGTERM, sig_handler);
	signal(SIGCHLD, sig_handler);
	signal(SIGPIPE, SIG_IGN);

	init_socket();
	init_xcb();

	exec_config(config_path);

	int max_fd = MAX(g_sock_fd, g_dpy_fd) + 1;
	fd_set file_descriptors;

	while (!g_is_about_to_quit) {
		xcb_flush(g_dpy);

		FD_ZERO(&file_descriptors);
		FD_SET(g_sock_fd, &file_descriptors);
		FD_SET(g_dpy_fd, &file_descriptors);

		select(max_fd, &file_descriptors, NULL, NULL, NULL);

		if (FD_ISSET(g_sock_fd, &file_descriptors)) {
			int client_fd;
			char msg[BUFSIZ];
			int msg_len;

			client_fd = accept(g_sock_fd, NULL, 0);

			if (!(client_fd < 0) && (msg_len = read(client_fd, msg, sizeof(msg))) > 0) {
				// client sent something
				msg[msg_len] = '\0';
				// TODO: handle event
				close(client_fd);
			}
		}

		if (FD_ISSET(g_dpy_fd, &file_descriptors)) {
			xcb_generic_event_t *event = xcb_wait_for_event(g_dpy);

			if (!event) {
				die("XCB connection broke\n");
			}

			handle_xcb_event(event);
		}
	}
}

static void
exec_config(const char *path)
{
	switch(fork()) {
	case -1:
		warn("failed to execute config\n");
		break;
	case 0:
		execl(path, path, NULL);
		die("failed to execute config\n");
	}
}

static void
sig_handler(int sig)
{
	if (sig == SIGCHLD) {
		signal(sig, sig_handler);
		while (waitpid(-1, 0, WNOHANG) > 0) {
			// EMPTY
		}
	} else if (sig == SIGINT || sig == SIGHUP || sig == SIGTERM) {
		g_is_about_to_quit = 1;
	}
}
