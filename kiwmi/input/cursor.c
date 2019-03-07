/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "kiwmi/input/cursor.h"

#include <stdlib.h>

#include <wayland-server.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/util/log.h>

static void
cursor_motion_notify(struct wl_listener *listener, void *data)
{
    struct kiwmi_cursor *cursor =
        wl_container_of(listener, cursor, cursor_motion);
    struct wlr_event_pointer_motion *event = data;

    wlr_cursor_move(
        cursor->cursor, event->device, event->delta_x, event->delta_y);
}

static void
cursor_motion_absolute_notify(struct wl_listener *listener, void *data)
{
    struct kiwmi_cursor *cursor =
        wl_container_of(listener, cursor, cursor_motion);
    struct wlr_event_pointer_motion_absolute *event = data;

    wlr_cursor_warp_absolute(cursor->cursor, event->device, event->x, event->y);
}

struct kiwmi_cursor *
cursor_create(struct wlr_output_layout *output_layout)
{
    wlr_log(WLR_DEBUG, "Creating cursor");

    struct kiwmi_cursor *cursor = malloc(sizeof(*cursor));
    if (!cursor) {
        wlr_log(WLR_ERROR, "Failed to allocate kiwmi_cursor");
        return NULL;
    }

    cursor->cursor = wlr_cursor_create();
    if (!cursor->cursor) {
        wlr_log(WLR_ERROR, "Failed to create cursor");
        free(cursor);
        return NULL;
    }

    wlr_cursor_attach_output_layout(cursor->cursor, output_layout);

    cursor->xcursor_manager = wlr_xcursor_manager_create(NULL, 24);
    wlr_xcursor_manager_load(cursor->xcursor_manager, 1);

    cursor->cursor_motion.notify = cursor_motion_notify;
    wl_signal_add(&cursor->cursor->events.motion, &cursor->cursor_motion);

    cursor->cursor_motion_absolute.notify = cursor_motion_absolute_notify;
    wl_signal_add(
        &cursor->cursor->events.motion_absolute,
        &cursor->cursor_motion_absolute);

    return cursor;
}
