/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef KIWMI_DESKTOP_OUTPUT_H
#define KIWMI_DESKTOP_OUTPUT_H

#include <wayland-server.h>
#include <wlr/types/wlr_output.h>

#include "kiwmi/desktop/desktop.h"

struct kiwmi_output {
    struct wl_list link;
    struct kiwmi_desktop *desktop;
    struct wlr_output *wlr_output;
    struct wl_listener frame;
    struct wl_listener destroy;
};

void new_output_notify(struct wl_listener *listener, void *data);

#endif /* KIWMI_DESKTOP_OUTPUT_H */
