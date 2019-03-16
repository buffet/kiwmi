/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "kiwmi/server.h"

#include <stdlib.h>

#include <wayland-server.h>
#include <wlr/backend.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/util/log.h>

#include "kiwmi/desktop/output.h"
#include "kiwmi/input.h"
#include "kiwmi/input/cursor.h"

bool
server_init(struct kiwmi_server *server)
{
    wlr_log(WLR_DEBUG, "Initializing Wayland server");

    server->wl_display = wl_display_create();
    server->backend    = wlr_backend_autocreate(server->wl_display, NULL);
    if (!server->backend) {
        wlr_log(WLR_ERROR, "Failed to create backend");
        wl_display_destroy(server->wl_display);
        return false;
    }

    struct wlr_renderer *renderer = wlr_backend_get_renderer(server->backend);
    wlr_renderer_init_wl_display(renderer, server->wl_display);

    server->compositor = wlr_compositor_create(server->wl_display, renderer);
    server->data_device_manager =
        wlr_data_device_manager_create(server->wl_display);

    server->output_layout = wlr_output_layout_create();

    server->cursor = cursor_create(server->output_layout);
    if (!server->cursor) {
        wlr_log(WLR_ERROR, "Failed to create cursor");
        wlr_backend_destroy(server->backend);
        wl_display_destroy(server->wl_display);
        return false;
    }

    wl_list_init(&server->keyboards);
    wl_list_init(&server->outputs);

    server->new_output.notify = new_output_notify;
    wl_signal_add(&server->backend->events.new_output, &server->new_output);

    server->new_input.notify = new_input_notify;
    wl_signal_add(&server->backend->events.new_input, &server->new_input);

    return true;
}

bool
server_run(struct kiwmi_server *server)
{
    server->socket = wl_display_add_socket_auto(server->wl_display);
    if (!server->socket) {
        wlr_log(WLR_ERROR, "Failed to open Wayland socket");
        wlr_backend_destroy(server->backend);
        wl_display_destroy(server->wl_display);
        return false;
    }

    wlr_log(
        WLR_DEBUG, "Running Wayland server on display '%s'", server->socket);

    if (!wlr_backend_start(server->backend)) {
        wlr_log(WLR_ERROR, "Failed to start backend");
        wlr_backend_destroy(server->backend);
        wl_display_destroy(server->wl_display);
        return false;
    }

    setenv("WAYLAND_DISPLAY", server->socket, true);

    wl_display_run(server->wl_display);

    return true;
}

void
server_fini(struct kiwmi_server *server)
{
    wlr_log(WLR_DEBUG, "Shutting down Wayland server");

    wlr_backend_destroy(server->backend);
    wl_display_destroy_clients(server->wl_display);
    wl_display_destroy(server->wl_display);
}
