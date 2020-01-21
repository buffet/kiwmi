/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef KIWMI_DESKTOP_XDG_SHELL_H
#define KIWMI_DESKTOP_XDG_SHELL_H

#include <wayland-server.h>

struct kiwmi_xdg_decoration {
    struct kiwmi_view *view;
    struct wlr_xdg_toplevel_decoration_v1 *wlr_decoration;

    struct wl_listener destroy;
    struct wl_listener request_mode;
};

void xdg_shell_new_surface_notify(struct wl_listener *listener, void *data);
void
xdg_toplevel_new_decoration_notify(struct wl_listener *listener, void *data);

#endif /* KIWMI_DESKTOP_XDG_SHELL_H */
