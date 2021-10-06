/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <limits.h>
#include <signal.h>
#include <unistd.h>

#include <wlr/util/log.h>

#include "server.h"

int
main(int argc, char **argv)
{
    int verbosity     = 0;
    char *config_path = NULL;

    const char *usage =
        "Usage: kiwmi [options]\n"
        "\n"
        "  -h  Show help message and exit\n"
        "  -v  Show version number and exit\n"
        "  -c  Change config path\n"
        "  -V  Increase verbosity level\n";

    int option;
    while ((option = getopt(argc, argv, "hvc:V")) != -1) {
        switch (option) {
        case 'h':
            printf("%s", usage);
            exit(EXIT_SUCCESS);
            break;
        case 'v':
            printf("kiwmi version " KIWMI_VERSION "\n");
            exit(EXIT_SUCCESS);
            break;
        case 'c':
            config_path = strdup(optarg); // gets freed in server_fini
            if (!config_path) {
                fprintf(stderr, "Failed to allocate memory\n");
                exit(EXIT_FAILURE);
            }
            break;
        case 'V':
            ++verbosity;
            break;
        default:
            fprintf(stderr, "%s", usage);
            exit(EXIT_FAILURE);
        }
    }

    switch (verbosity) {
    case 0:
        wlr_log_init(WLR_ERROR, NULL);
        break;
    case 1:
        wlr_log_init(WLR_INFO, NULL);
        break;
    default:
        wlr_log_init(WLR_DEBUG, NULL);
    }

    fprintf(stderr, "Using kiwmi v" KIWMI_VERSION "\n");

    signal(SIGCHLD, SIG_IGN); // prevent zombies from kiwmi:spawn()

    if (!getenv("XDG_RUNTIME_DIR")) {
        wlr_log(WLR_ERROR, "XDG_RUNTIME_DIR not set");
        exit(EXIT_FAILURE);
    }

    struct kiwmi_server server;

    if (!server_init(&server, config_path)) {
        wlr_log(WLR_ERROR, "Failed to initialize server");
        free(config_path);
        exit(EXIT_FAILURE);
    }

    if (!server_run(&server)) {
        wlr_log(WLR_ERROR, "Failed to run server");
        exit(EXIT_FAILURE);
    }

    server_fini(&server);
}
