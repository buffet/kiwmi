#include "kiwmi/server.h"

#include <stdlib.h>

#include <wayland-server.h>
#include <wlr/backend.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/util/log.h>

bool
server_init(struct kiwmi_server *server)
{
    wlr_log(WLR_DEBUG, "Initializing Wayland server");

    server->wl_display = wl_display_create();
    server->backend = wlr_backend_autocreate(server->wl_display, NULL);
    if (!server->backend) {
        wlr_log(WLR_ERROR, "Failed to create backend");
        return false;
    }

    struct wlr_renderer *renderer = wlr_backend_get_renderer(server->backend);
    wlr_renderer_init_wl_display(renderer, server->wl_display);

    server->compositor = wlr_compositor_create(server->wl_display, renderer);
    server->data_device_manager = wlr_data_device_manager_create(server->wl_display);

    server->output_layout = wlr_output_layout_create();

    server->socket = wl_display_add_socket_auto(server->wl_display);
    if (!server->socket) {
        wlr_log(WLR_ERROR, "Failed to open Wayland socket");
        wlr_backend_destroy(server->backend);
        wl_display_destroy(server->wl_display);
        return false;
    }

    if (!wlr_backend_start(server->backend)) {
        wlr_log(WLR_ERROR, "Failed to start backend");
        wlr_backend_destroy(server->backend);
        wl_display_destroy(server->wl_display);
        return false;
    }

    setenv("WAYLAND_DISPLAY", server->socket, true);

    return true;
}

void
server_run(struct kiwmi_server *server)
{
    wlr_log(WLR_DEBUG, "Running Wayland server on display '%s'", server->socket);

    wl_display_run(server->wl_display);
}

void
server_fini(struct kiwmi_server *server)
{
    wlr_log(WLR_DEBUG, "Shutting down Wayland server");

    wlr_backend_destroy(server->backend);
    wl_display_destroy_clients(server->wl_display);
    wl_display_destroy(server->wl_display);
}
