/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef KIWMI_INPUT_SEAT_H
#define KIWMI_INPUT_SEAT_H

#include <wayland-server.h>

#include "input/input.h"

struct kiwmi_seat {
    struct kiwmi_input *input;
    struct wlr_seat *seat;

    struct wl_listener request_set_cursor;
};

struct kiwmi_seat *seat_create(struct kiwmi_input *input);
void seat_destroy(struct kiwmi_seat *seat);

#endif /* KIWMI_INPUT_SEAT_H */
