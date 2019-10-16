/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef KIWMI_INPUT_KEYBOARD_H
#define KIWMI_INPUT_KEYBOARD_H

#include <wayland-server.h>

struct kiwmi_keyboard {
    struct wl_list link;
    struct kiwmi_server *server;
    struct wlr_input_device *device;
    struct wl_listener modifiers;
    struct wl_listener key;
};

struct kiwmi_keyboard *
keyboard_create(struct kiwmi_server *server, struct wlr_input_device *device);

#endif /* KIWMI_INPUT_KEYBOARD_H */
