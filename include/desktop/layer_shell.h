/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef KIWMI_DESKTOP_LAYER_SHELL_H
#define KIWMI_DESKTOP_LAYER_SHELL_H

#include <stdbool.h>

#include <wayland-server.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/util/box.h>

#include "desktop/desktop_surface.h"
#include "desktop/output.h"

struct kiwmi_layer {
    struct wl_list link;
    struct kiwmi_desktop_surface desktop_surface;

    struct wlr_layer_surface_v1 *layer_surface;
    uint32_t layer; // enum zwlr_layer_shell_v1_layer

    struct kiwmi_output *output;

    struct wl_listener destroy;
    struct wl_listener commit;
    struct wl_listener map;
    struct wl_listener unmap;
};

void arrange_layers(struct kiwmi_output *output);

void layer_shell_new_surface_notify(struct wl_listener *listener, void *data);

#endif /* KIWMI_DESKTOP_LAYER_SHELL_H */
