/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef KIWMI_INPUT_KEYBOARD_H
#define KIWMI_INPUT_KEYBOARD_H

#include <stdint.h>

#include <wayland-server.h>
#include <xkbcommon/xkbcommon.h>

struct kiwmi_keyboard {
    struct wl_list link;
    struct kiwmi_server *server;
    struct wlr_keyboard *wlr_keyboard;
    struct wl_listener modifiers;
    struct wl_listener key;
    struct wl_listener device_destroy;

    struct {
        struct wl_signal key_down;
        struct wl_signal key_up;
        struct wl_signal destroy;
    } events;
};

struct kiwmi_keyboard_key_event {
    const xkb_keysym_t *raw_syms;
    const xkb_keysym_t *translated_syms;
    int raw_syms_len;
    int translated_syms_len;
    uint32_t keycode;
    struct kiwmi_keyboard *keyboard;
    bool handled;
};

struct kiwmi_keyboard *
keyboard_create(struct kiwmi_server *server, struct wlr_keyboard *device);
void keyboard_destroy(struct kiwmi_keyboard *keyboard);

#endif /* KIWMI_INPUT_KEYBOARD_H */
