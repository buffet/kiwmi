/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef KIWMI_INPUT_SEAT_H
#define KIWMI_INPUT_SEAT_H

#include <wayland-server.h>
#include <wlr/types/wlr_surface.h>

#include "desktop/layer_shell.h"
#include "desktop/view.h"
#include "input/input.h"

struct kiwmi_seat {
    struct kiwmi_input *input;
    struct wlr_seat *seat;

    struct kiwmi_view *focused_view;
    struct kiwmi_layer *focused_layer;

    struct wl_listener request_set_cursor;
    struct wl_listener request_set_selection;
    struct wl_listener request_set_primary_selection;
};

void
seat_focus_surface(struct kiwmi_seat *seat, struct wlr_surface *wlr_surface);
void seat_focus_layer(struct kiwmi_seat *seat, struct kiwmi_layer *layer);
void seat_focus_view(struct kiwmi_seat *seat, struct kiwmi_view *view);

struct kiwmi_seat *seat_create(struct kiwmi_input *input);
void seat_destroy(struct kiwmi_seat *seat);

#endif /* KIWMI_INPUT_SEAT_H */
