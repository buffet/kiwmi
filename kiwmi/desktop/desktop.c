/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "kiwmi/desktop/desktop.h"

#include <stdbool.h>

#include <wayland-server.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/util/log.h>

#include "kiwmi/server.h"
#include "kiwmi/desktop/output.h"
#include "kiwmi/input/cursor.h"

bool
desktop_init(struct kiwmi_desktop *desktop, struct wlr_renderer *renderer)
{
    struct kiwmi_server *server = wl_container_of(desktop, server, desktop);
    desktop->compositor = wlr_compositor_create(server->wl_display, renderer);
    desktop->data_device_manager =
        wlr_data_device_manager_create(server->wl_display);
    desktop->output_layout = wlr_output_layout_create();

    desktop->cursor = cursor_create(desktop->output_layout);
    if (!desktop->cursor) {
        wlr_log(WLR_ERROR, "Failed to create cursor");
        return false;
    }

    wl_list_init(&desktop->outputs);

    desktop->new_output.notify = new_output_notify;
    wl_signal_add(&server->backend->events.new_output, &desktop->new_output);

    return true;
}
