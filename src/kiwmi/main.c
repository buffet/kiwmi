#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <getopt.h>
#include <limits.h>

#include "common.h"

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
}
