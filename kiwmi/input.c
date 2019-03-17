/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "kiwmi/input.h"

#include <wayland-server.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/util/log.h>

#include "kiwmi/server.h"
#include "kiwmi/input/input.h"
#include "kiwmi/input/keyboard.h"

static void
new_pointer(struct kiwmi_input *input, struct wlr_input_device *device)
{
    struct kiwmi_server *server = wl_container_of(input, server, input);

    wlr_cursor_attach_input_device(server->desktop.cursor->cursor, device);
}

static void
new_keyboard(struct kiwmi_input *input, struct wlr_input_device *device)
{
    struct kiwmi_server *server = wl_container_of(input, server, input);

    struct kiwmi_keyboard *keyboard = keyboard_create(server, device);
    if (!keyboard) {
        return;
    }

    wl_list_insert(&input->keyboards, &keyboard->link);
}

void
new_input_notify(struct wl_listener *listener, void *data)
{
    struct kiwmi_input *input = wl_container_of(listener, input, new_input);
    struct wlr_input_device *device = data;

    wlr_log(WLR_DEBUG, "New input %p: %s", device, device->name);

    switch (device->type) {
    case WLR_INPUT_DEVICE_POINTER:
        new_pointer(input, device);
        break;
    case WLR_INPUT_DEVICE_KEYBOARD:
        new_keyboard(input, device);
        break;
    default:
        // NOT HANDLED
        break;
    }
}
