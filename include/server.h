/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef KIWMI_SERVER_H
#define KIWMI_SERVER_H

#include <stdbool.h>

#include "desktop/desktop.h"
#include "input/input.h"

struct kiwmi_server {
    struct wl_display *wl_display;
    struct wl_event_loop *wl_event_loop;
    struct wlr_backend *backend;
    const char *socket;
    char *config_path;
    struct kiwmi_desktop desktop;
    struct kiwmi_input input;
};

bool server_init(struct kiwmi_server *server, char *config_path);
bool server_run(struct kiwmi_server *server);
void server_fini(struct kiwmi_server *server);

#endif /* KIWMI_SERVER_H */
