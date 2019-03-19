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

#include <wayland-server.h>
#include <wlr/util/log.h>

#include "kiwmi/server.h"

int
main(int argc, char **argv)
{
    int verbosity             = 0;
    const char *frontend_path = NULL;

    const char *usage =
        "Usage: kiwmi [options] FRONTEND\n"
        "\n"
        "  -h, --help     Show help message and quit\n"
        "  -v, --version  Show version number and quit\n"
        "  -V, --verbose  Increase verbosity Level\n";

    int option;
    while ((option = getopt(argc, argv, "hvV")) != -1) {
        switch (option) {
        case 'h':
            printf("%s", usage);
            exit(EXIT_SUCCESS);
            break;
        case 'v':
            printf("kiwmi version " KIWMI_VERSION "\n");
            exit(EXIT_SUCCESS);
            break;
        case 'V':
            ++verbosity;
            break;
        default:
            fprintf(stderr, "%s", usage);
            exit(EXIT_FAILURE);
        }
    }

    // no frontend passsed
    if (optind >= argc) {
        fprintf(stderr, "%s", usage);
        exit(EXIT_FAILURE);
    }

    frontend_path = argv[optind];

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

    struct kiwmi_server server;

    if (!server_init(&server)) {
        wlr_log(WLR_ERROR, "Failed to initialize server");
        exit(EXIT_FAILURE);
    }

    if (!server_run(&server)) {
        wlr_log(WLR_ERROR, "Failed to run server");
        exit(EXIT_FAILURE);
    }

    server_fini(&server);
}
