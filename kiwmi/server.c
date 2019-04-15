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
#include <wlr/util/log.h>

bool
server_init(struct kiwmi_server *server, const char *frontend_path)
{
    wlr_log(WLR_DEBUG, "Initializing Wayland server");

    server->wl_display    = wl_display_create();
    server->wl_event_loop = wl_display_get_event_loop(server->wl_display);
    server->backend       = wlr_backend_autocreate(server->wl_display, NULL);
    if (!server->backend) {
        wlr_log(WLR_ERROR, "Failed to create backend");
        wl_display_destroy(server->wl_display);
        return false;
    }

    struct wlr_renderer *renderer = wlr_backend_get_renderer(server->backend);
    wlr_renderer_init_wl_display(renderer, server->wl_display);

    if (!desktop_init(&server->desktop, renderer)) {
        wlr_log(WLR_ERROR, "Failed to initialize desktop");
        wl_display_destroy(server->wl_display);
        return false;
    }

    if (!input_init(&server->input)) {
        wlr_log(WLR_ERROR, "Failed to initialize input");
        wl_display_destroy(server->wl_display);
        return false;
    }

    server->socket = wl_display_add_socket_auto(server->wl_display);
    if (!server->socket) {
        wlr_log(WLR_ERROR, "Failed to open Wayland socket");
        wl_display_destroy(server->wl_display);
        return false;
    }

    if (!frontend_init(&server->frontend, frontend_path)) {
        wlr_log(WLR_ERROR, "Failed to initialize frontend");
        wl_display_destroy(server->wl_display);
        return false;
    }

    return true;
}

bool
server_run(struct kiwmi_server *server)
{
    wlr_log(
        WLR_DEBUG, "Running Wayland server on display '%s'", server->socket);

    if (!wlr_backend_start(server->backend)) {
        wlr_log(WLR_ERROR, "Failed to start backend");
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

    wl_display_destroy_clients(server->wl_display);
    wl_display_destroy(server->wl_display);
}
