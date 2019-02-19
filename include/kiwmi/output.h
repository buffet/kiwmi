/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef KIWMI_OUTPUT_H
#define KIWMI_OUTPUT_H

#include <wayland-server.h>
#include <wlr/types/wlr_output.h>

#include "kiwmi/server.h"

struct kiwmi_output {
    struct wl_list link;
    struct kiwmi_server *server;
    struct wlr_output *wlr_output;
    struct wl_listener frame;
    struct wl_listener destroy;
};

struct kiwmi_output *
output_create(struct wlr_output *wlr_output, struct kiwmi_server *server);

#endif /* KIWMI_OUTPUT_H */
