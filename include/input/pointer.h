/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef KIWMI_INPUT_POINTER_H
#define KIWMI_INPUT_POINTER_H

#include <wayland-server.h>
#include <wlr/types/wlr_pointer.h>

#include "server.h"

struct kiwmi_pointer {
    struct wlr_pointer *pointer;
    struct wl_list link;

    struct wl_listener device_destroy;
};

struct kiwmi_pointer *
pointer_create(struct kiwmi_server *server, struct wlr_pointer *pointer);
void pointer_destroy(struct kiwmi_pointer *pointer);

#endif /* KIWMI_INPUT_POINTER_H */
