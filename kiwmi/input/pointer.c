/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "input/pointer.h"

#include <stdlib.h>
#include <string.h>

#include <wayland-server.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_input_device.h>

#include "desktop/output.h"
#include "input/cursor.h"
#include "input/input.h"
#include "server.h"

static void
pointer_destroy_notify(struct wl_listener *listener, void *UNUSED(data))
{
    struct kiwmi_pointer *pointer =
        wl_container_of(listener, pointer, device_destroy);

    pointer_destroy(pointer);
}

struct kiwmi_pointer *
pointer_create(struct kiwmi_server *server, struct wlr_pointer *device)
{
    wlr_cursor_attach_input_device(server->input.cursor->cursor, &device->base);

    struct kiwmi_pointer *pointer = malloc(sizeof(*pointer));
    if (!pointer) {
        return NULL;
    }

    pointer->pointer = device;

    pointer->device_destroy.notify = pointer_destroy_notify;
    wl_signal_add(&device->base.events.destroy, &pointer->device_destroy);

    // FIXME: `wlr_input_device` doesn't contain `output_name`
    if (device->output_name) {
        struct kiwmi_output *output;
        wl_list_for_each (output, &server->desktop.outputs, link) {
            if (strcmp(device->output_name, output->wlr_output->name) == 0) {
                wlr_cursor_map_input_to_output(
                    server->input.cursor->cursor,
                    &device->base,
                    output->wlr_output);
                break;
            }
        }
    }

    return pointer;
}

void
pointer_destroy(struct kiwmi_pointer *pointer)
{
    wl_list_remove(&pointer->device_destroy.link);
    wl_list_remove(&pointer->link);

    free(pointer);
}
