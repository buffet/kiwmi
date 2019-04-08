/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef KIWMI_FRONTEND_H
#define KIWMI_FRONTEND_H

#include <stdbool.h>
#include <stdio.h>

#include <wayland-server.h>

struct kiwmi_frontend {
    const char *frontend_path;
    int sock_fd;
    char *sock_path;
    struct wl_event_source *sock_event_source;
    struct wl_listener display_destroy;
};

bool frontend_init(struct kiwmi_frontend *frontend, const char *frontend_path);

#endif /* KIWMI_FRONTEND_H */
