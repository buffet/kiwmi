/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef KIWMI_DESKTOP_XDG_SHELL_H
#define KIWMI_DESKTOP_XDG_SHELL_H

#include <wayland-server.h>

void xdg_shell_new_surface_notify(struct wl_listener *listener, void *data);

#endif /* KIWMI_DESKTOP_XDG_SHELL_H */
