/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "desktop/desktop.h"

#include <stdbool.h>

#include <wayland-server.h>
#include <wlr/backend.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_export_dmabuf_v1.h>
#include <wlr/types/wlr_layer_shell_v1.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_xdg_decoration_v1.h>
#include <wlr/types/wlr_xdg_output_v1.h>
#include <wlr/types/wlr_xdg_shell.h>

#include "desktop/layer_shell.h"
#include "desktop/output.h"
#include "desktop/view.h"
#include "desktop/xdg_shell.h"
#include "input/cursor.h"
#include "input/input.h"
#include "server.h"

bool
desktop_init(struct kiwmi_desktop *desktop)
{
    struct kiwmi_server *server = wl_container_of(desktop, server, desktop);

    desktop->compositor =
        wlr_compositor_create(server->wl_display, server->renderer);
    desktop->data_device_manager =
        wlr_data_device_manager_create(server->wl_display);
    desktop->output_layout = wlr_output_layout_create();

    wlr_export_dmabuf_manager_v1_create(server->wl_display);
    wlr_xdg_output_manager_v1_create(
        server->wl_display, desktop->output_layout);

    desktop->bg_color[0] = 0.1f;
    desktop->bg_color[1] = 0.1f;
    desktop->bg_color[2] = 0.1f;
    desktop->bg_color[3] = 1.0f;

    desktop->xdg_shell = wlr_xdg_shell_create(server->wl_display);
    desktop->xdg_shell_new_surface.notify = xdg_shell_new_surface_notify;
    wl_signal_add(
        &desktop->xdg_shell->events.new_surface,
        &desktop->xdg_shell_new_surface);

    desktop->xdg_decoration_manager =
        wlr_xdg_decoration_manager_v1_create(server->wl_display);
    desktop->xdg_toplevel_new_decoration.notify =
        xdg_toplevel_new_decoration_notify;
    wl_signal_add(
        &desktop->xdg_decoration_manager->events.new_toplevel_decoration,
        &desktop->xdg_toplevel_new_decoration);

    desktop->layer_shell = wlr_layer_shell_v1_create(server->wl_display);
    desktop->layer_shell_new_surface.notify = layer_shell_new_surface_notify;
    wl_signal_add(
        &desktop->layer_shell->events.new_surface,
        &desktop->layer_shell_new_surface);

    wl_list_init(&desktop->outputs);
    wl_list_init(&desktop->views);

    desktop->new_output.notify = new_output_notify;
    wl_signal_add(&server->backend->events.new_output, &desktop->new_output);

    wl_signal_init(&desktop->events.new_output);
    wl_signal_init(&desktop->events.view_map);
    wl_signal_init(&desktop->events.request_active_output);

    return true;
}

void
desktop_fini(struct kiwmi_desktop *desktop)
{
    wlr_output_layout_destroy(desktop->output_layout);
    desktop->output_layout = NULL;
}

struct kiwmi_output *
desktop_active_output(struct kiwmi_server *server)
{
    // 1. callback (request_active_output)
    struct kiwmi_output *output = NULL;
    wl_signal_emit(&server->desktop.events.request_active_output, &output);

    if (output) {
        return output;
    }

    // 2. focused view center
    if (!wl_list_empty(&server->desktop.views)) {
        struct kiwmi_view *view;
        wl_list_for_each (view, &server->desktop.views, link) {
            break; // get first element of list
        }

        double lx = view->x + view->geom.width / 2;
        double ly = view->y + view->geom.height / 2;

        struct wlr_output *wlr_output =
            wlr_output_layout_output_at(server->desktop.output_layout, lx, ly);
        return wlr_output->data;
    }

    // 3. cursor
    double lx = server->input.cursor->cursor->x;
    double ly = server->input.cursor->cursor->y;

    struct wlr_output *wlr_output =
        wlr_output_layout_output_at(server->desktop.output_layout, lx, ly);
    return wlr_output->data;
}
