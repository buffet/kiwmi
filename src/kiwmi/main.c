#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <getopt.h>
#include <limits.h>
#include <signal.h>
#include <sys/wait.h>

#include "common.h"

static void sig_handler(int sig);

int g_is_about_to_quit = 0;

int
main(int argc, char *argv[])
{
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
