/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "kiwmi/commands.h"

#include <string.h>

typedef bool (
    *cmd_handler)(FILE *client, const char **args, struct kiwmi_server *server);

static bool
cmd_quit(
    FILE *UNUSED(client),
    const char **UNUSED(args),
    struct kiwmi_server *server)
{
    wl_display_terminate(server->wl_display);
    return false;
}

static const struct {
    const char *name;
    cmd_handler handler;
} commands[] = {
    {"quit", cmd_quit},
};

bool
handle_client_command(char *command, FILE *client, struct kiwmi_server *server)
{
#define SIZE(arr) (sizeof((arr)) / sizeof(*(arr)))

    const char *name = strtok(command, " \t\r");

    for (size_t i = 0; i < SIZE(commands); ++i) {
        if (strcmp(name, commands[i].name) == 0) {
            return commands[i].handler(client, NULL, server);
        }
    }

    fprintf(client, "Unknown command: %s\n", name);

    return false;

#undef SIZE
}
