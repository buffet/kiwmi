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

#include <wayland-server.h>
#include <wlr/util/log.h>

#include "kiwmi/server.h"

struct kiwmi_config {
    int verbosity;
    const char *frontend_path;
};

struct option {
    const char *name;
    int val;
};

static void
parse_arguments(int argc, char **argv, struct kiwmi_config *config)
{
    static struct option options[] = {
        {"help", 'h'},
        {"version", 'v'},
        {"verbose", 'V'},

        {0, 0},
    };

    const char *usage =
        "Usage: kiwmi [options] FRONTEND\n"
        "\n"
        "  -h, --help     Show help message and quit\n"
        "  -v, --version  Show version number and quit\n"
        "  -V, --verbose  Increase verbosity Level\n";

    config->verbosity     = 0;
    config->frontend_path = NULL;

    for (int i = 1; i < argc; ++i) {
        const char *opt = argv[i];
        char val        = '?';

        if (opt[0] == '-') {
            if (opt[1] == '-') {
                for (const struct option *current = options; current->name;
                     ++current) {
                    if (strcmp(&opt[2], current->name)) {
                        val = current->val;
                        break;
                    }
                }
            } else {
                val = opt[1];
            }

            switch (val) {
            case 'h':
                printf("%s", usage);
                exit(EXIT_SUCCESS);
                break;
            case 'v':
                printf("kiwmi version " KIWMI_VERSION "\n");
                exit(EXIT_SUCCESS);
                break;
            case 'V':
                ++config->verbosity;
                break;
            default:
                fprintf(stderr, "%s", usage);
                exit(EXIT_FAILURE);
            }
        } else {
            config->frontend_path = opt;
            break;
        }
    }

    if (!config->frontend_path) {
        fprintf(stderr, "%s", usage);
        exit(EXIT_FAILURE);
    }
}

int
main(int argc, char **argv)
{
    struct kiwmi_config config;
    parse_arguments(argc, argv, &config);

    switch (config.verbosity) {
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
