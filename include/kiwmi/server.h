/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef KIWMI_SERVER_H
#define KIWMI_SERVER_H

#include <wayland-server.h>
#include <wlr/backend.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_output_layout.h>

#include "kiwmi/input/cursor.h"

struct kiwmi_server {
    struct wl_display *wl_display;
    struct wlr_backend *backend;
    struct wlr_compositor *compositor;
    struct wlr_data_device_manager *data_device_manager;
    struct wlr_output_layout *output_layout;
    const char *socket;
    struct wl_list outputs; // struct kiwmi_output::link
    struct wl_listener new_output;
    struct kiwmi_cursor *cursor;
    struct wl_listener new_input;
};

bool server_init(struct kiwmi_server *server);
bool server_run(struct kiwmi_server *server);
void server_fini(struct kiwmi_server *server);

#endif /* KIWMI_SERVER_H */
