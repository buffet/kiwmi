/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "input/input.h"

#include <stdlib.h>

#include <wayland-server.h>
#include <wlr/backend.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/util/log.h>

#include "desktop/desktop.h"
#include "input/cursor.h"
#include "input/keyboard.h"
#include "input/pointer.h"
#include "input/seat.h"
#include "server.h"

static void
new_pointer(struct kiwmi_input *input, struct wlr_pointer *device)
{
    struct kiwmi_server *server = wl_container_of(input, server, input);

    struct kiwmi_pointer *pointer = pointer_create(server, device);
    if (!pointer) {
        return;
    }

    wl_list_insert(&input->pointers, &pointer->link);
}

static void
new_keyboard(struct kiwmi_input *input, struct wlr_keyboard *device)
{
    struct kiwmi_server *server = wl_container_of(input, server, input);

    struct kiwmi_keyboard *keyboard = keyboard_create(server, device);
    if (!keyboard) {
        return;
    }

    wl_list_insert(&input->keyboards, &keyboard->link);
}

static void
new_input_notify(struct wl_listener *listener, void *data)
{
    struct kiwmi_input *input = wl_container_of(listener, input, new_input);
    struct wlr_input_device *device = data;

    wlr_log(WLR_DEBUG, "New input %p: %s", device, device->name);

    switch (device->type) {
    case WLR_INPUT_DEVICE_POINTER: {
        struct wlr_pointer *pointer = wlr_pointer_from_input_device(device);
        new_pointer(input, pointer);
        break;
    }
    case WLR_INPUT_DEVICE_KEYBOARD: {
        struct wlr_keyboard *keyboard = wlr_keyboard_from_input_device(device);
        new_keyboard(input, keyboard);
        break;
    }
    default:
        // NOT HANDLED
        break;
    }

    uint32_t caps = WL_SEAT_CAPABILITY_POINTER;
    if (!wl_list_empty(&input->keyboards)) {
        caps |= WL_SEAT_CAPABILITY_KEYBOARD;
    }

    wlr_seat_set_capabilities(input->seat->seat, caps);
}

bool
input_init(struct kiwmi_input *input)
{
    struct kiwmi_server *server = wl_container_of(input, server, input);

    input->seat = seat_create(input);
    if (!input->seat) {
        return false;
    }

    input->cursor = cursor_create(server, server->desktop.output_layout);
    if (!input->cursor) {
        wlr_log(WLR_ERROR, "Failed to create cursor");
        return false;
    }

    wl_list_init(&input->keyboards);
    wl_list_init(&input->pointers);

    input->new_input.notify = new_input_notify;
    wl_signal_add(&server->backend->events.new_input, &input->new_input);

    wl_signal_init(&input->events.keyboard_new);

    return true;
}

void
input_fini(struct kiwmi_input *input)
{
    struct kiwmi_keyboard *keyboard;
    struct kiwmi_keyboard *tmp_keyboard;
    wl_list_for_each_safe (keyboard, tmp_keyboard, &input->keyboards, link) {
        keyboard_destroy(keyboard);
    }

    struct kiwmi_pointer *pointer;
    struct kiwmi_pointer *tmp_pointer;
    wl_list_for_each_safe (pointer, tmp_pointer, &input->pointers, link) {
        pointer_destroy(pointer);
    }

    seat_destroy(input->seat);

    cursor_destroy(input->cursor);
}
