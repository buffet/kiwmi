/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "input/keyboard.h"

#include <stdbool.h>
#include <stdlib.h>

#include <wayland-server.h>
#include <wlr/backend.h>
#include <wlr/backend/multi.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/util/log.h>
#include <xkbcommon/xkbcommon.h>

#include "server.h"

static bool
switch_vt(const xkb_keysym_t *syms, int nsyms, struct wlr_backend *backend)
{
    for (int i = 0; i < nsyms; ++i) {
        const xkb_keysym_t sym = syms[i];

        if (sym >= XKB_KEY_XF86Switch_VT_1 && sym <= XKB_KEY_XF86Switch_VT_12) {
            if (wlr_backend_is_multi(backend)) {
                struct wlr_session *session = wlr_backend_get_session(backend);
                if (session) {
                    unsigned vt = sym - XKB_KEY_XF86Switch_VT_1 + 1;
                    wlr_session_change_vt(session, vt);
                }
            }
            return true;
        }
    }

    return false;
}

static void
keyboard_modifiers_notify(
    struct wl_listener *UNUSED(listener),
    void *UNUSED(data))
{
}

static void
keyboard_key_notify(struct wl_listener *listener, void *data)
{
    struct kiwmi_keyboard *keyboard = wl_container_of(listener, keyboard, key);
    struct kiwmi_server *server     = keyboard->server;
    struct wlr_event_keyboard_key *event = data;

    uint32_t keycode = event->keycode + 8;
    const xkb_keysym_t *syms;
    int nsyms = xkb_state_key_get_syms(
        keyboard->device->keyboard->xkb_state, keycode, &syms);
    uint32_t modifiers = wlr_keyboard_get_modifiers(keyboard->device->keyboard);

    bool handled = false;

    if (event->state == WLR_KEY_PRESSED) {
        handled = switch_vt(syms, nsyms, server->backend);

        if (!handled && (modifiers & WLR_MODIFIER_LOGO)) {
            for (int i = 0; i < nsyms; ++i) {
                xkb_keysym_t sym = syms[i];

                switch (sym) {
                case XKB_KEY_Escape:
                    wl_display_terminate(server->wl_display);
                    handled = true;
                    break;
                }
            }
        }
    }
}

struct kiwmi_keyboard *
keyboard_create(struct kiwmi_server *server, struct wlr_input_device *device)
{
    wlr_log(WLR_DEBUG, "Creating keyboard");

    struct kiwmi_keyboard *keyboard = malloc(sizeof(*keyboard));
    if (!keyboard) {
        return NULL;
    }

    keyboard->server = server;
    keyboard->device = device;

    struct xkb_rule_names rules = {0};
    struct xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    struct xkb_keymap *keymap =
        xkb_map_new_from_names(context, &rules, XKB_KEYMAP_COMPILE_NO_FLAGS);

    keyboard->modifiers.notify = keyboard_modifiers_notify;
    wl_signal_add(&device->keyboard->events.modifiers, &keyboard->modifiers);

    keyboard->key.notify = keyboard_key_notify;
    wl_signal_add(&device->keyboard->events.key, &keyboard->key);

    wlr_keyboard_set_keymap(device->keyboard, keymap);
    xkb_keymap_unref(keymap);
    xkb_context_unref(context);
    wlr_keyboard_set_repeat_info(device->keyboard, 25, 600);

    return keyboard;
}
