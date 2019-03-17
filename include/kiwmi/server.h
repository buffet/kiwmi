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

#include "kiwmi/desktop/desktop.h"

struct kiwmi_server {
    struct wl_display *wl_display;
    struct wlr_backend *backend;
    const char *socket;
    struct kiwmi_desktop desktop;
    struct wl_list keyboards; // struct kiwmi_keyboard::link
    struct wl_listener new_input;
};

bool server_init(struct kiwmi_server *server);
bool server_run(struct kiwmi_server *server);
void server_fini(struct kiwmi_server *server);

#endif /* KIWMI_SERVER_H */
