/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <wayland-client.h>

#include "kiwmi-ipc-client-protocol.h"

static void
command_done(
    void *data,
    struct kiwmi_command *UNUSED(kiwmi_command),
    uint32_t error,
    const char *message)
{
    int *exit_code = data;
    FILE *out;

    if (error == KIWMI_COMMAND_ERROR_SUCCESS) {
        *exit_code = EXIT_SUCCESS;
        out        = stdout;
    } else {
        *exit_code = EXIT_FAILURE;
        out        = stderr;
    }

    if (message[0] != '\0') {
        fprintf(out, "%s\n", message);
    }
}

static const struct kiwmi_command_listener command_listener = {
    .done = command_done,
};

static void
registry_global(
    void *data,
    struct wl_registry *registry,
    uint32_t name,
    const char *interface,
    uint32_t UNUSED(version))
{
    struct kiwmi_ipc **ipc = data;
    if (strcmp(interface, kiwmi_ipc_interface.name) == 0) {
        *ipc = wl_registry_bind(registry, name, &kiwmi_ipc_interface, 1);
    }
}

static void
registry_global_remove(
    void *UNUSED(data),
    struct wl_registry *UNUSED(registry),
    uint32_t UNUSED(name))
{
    // EMPTY
}

static const struct wl_registry_listener registry_listener = {
    .global        = registry_global,
    .global_remove = registry_global_remove,
};

int
main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "Usage: kiwmic COMMAND\n");
        exit(EXIT_FAILURE);
    }

    struct wl_display *display = wl_display_connect(NULL);
    if (!display) {
        fprintf(stderr, "Failed to connect to display\n");
        exit(EXIT_FAILURE);
    }

    struct wl_registry *registry = wl_display_get_registry(display);
    struct kiwmi_ipc *ipc        = NULL;

    wl_registry_add_listener(registry, &registry_listener, &ipc);
    wl_display_roundtrip(display);

    if (!ipc) {
        fprintf(stderr, "Failed to bind to kiwmi_ipc\n");
        exit(EXIT_FAILURE);
    }

    struct kiwmi_command *command = kiwmi_ipc_eval(ipc, argv[1]);
    int exit_code;
    kiwmi_command_add_listener(command, &command_listener, &exit_code);
    wl_display_roundtrip(display);
    wl_display_disconnect(display);

    exit(exit_code);
}
