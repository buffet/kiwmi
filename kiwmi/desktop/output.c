/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "kiwmi/desktop/output.h"

#include <wayland-server.h>
#include <wlr/types/wlr_output.h>
#include <wlr/util/log.h>

#include "kiwmi/output.h"
#include "kiwmi/server.h"
#include "kiwmi/desktop/desktop.h"

void
new_output_notify(struct wl_listener *listener, void *data)
{
    struct kiwmi_desktop *desktop =
        wl_container_of(listener, desktop, new_output);
    struct kiwmi_server *server   = wl_container_of(desktop, server, desktop);
    struct wlr_output *wlr_output = data;

    wlr_log(WLR_DEBUG, "New output %p: %s", wlr_output, wlr_output->name);

    if (!wl_list_empty(&wlr_output->modes)) {
        struct wlr_output_mode *mode =
            wl_container_of(wlr_output->modes.prev, mode, link);
        wlr_output_set_mode(wlr_output, mode);
    }

    struct kiwmi_output *output = output_create(wlr_output, desktop);
    if (!output) {
        wlr_log(WLR_ERROR, "Failed to create output");
        return;
    }

    struct kiwmi_cursor *cursor = desktop->cursor;

    wlr_xcursor_manager_load(cursor->xcursor_manager, wlr_output->scale);

    wl_list_insert(&desktop->outputs, &output->link);

    wlr_output_layout_add_auto(desktop->output_layout, wlr_output);

    wlr_output_create_global(wlr_output);
}
