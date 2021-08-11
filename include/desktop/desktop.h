/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef KIWMI_DESKTOP_DESKTOP_H
#define KIWMI_DESKTOP_DESKTOP_H

#include <wayland-server.h>
#include <wlr/render/wlr_renderer.h>

#include "desktop/view.h"

struct kiwmi_desktop {
    struct wlr_compositor *compositor;
    struct wlr_xdg_shell *xdg_shell;
    struct wlr_xdg_decoration_manager_v1 *xdg_decoration_manager;
    struct wlr_layer_shell_v1 *layer_shell;
    struct wlr_data_device_manager *data_device_manager;
    struct wlr_output_layout *output_layout;
    struct wl_list outputs; // struct kiwmi_output::link
    struct wl_list views;   // struct kiwmi_view::link

    struct wl_listener xdg_shell_new_surface;
    struct wl_listener xdg_toplevel_new_decoration;
    struct wl_listener layer_shell_new_surface;
    struct wl_listener new_output;

    struct {
        struct wl_signal new_output;
        struct wl_signal view_map;
        struct wl_signal request_active_output;
    } events;
};

void desktop_raise_view(
    struct kiwmi_desktop *desktop,
    struct kiwmi_view *view,
    bool raise);
bool desktop_init(struct kiwmi_desktop *desktop, struct wlr_renderer *renderer);
void desktop_fini(struct kiwmi_desktop *desktop);

struct kiwmi_server;
struct kiwmi_output *desktop_active_output(struct kiwmi_server *server);

#endif /* KIWMI_DESKTOP_DESKTOP_H */
