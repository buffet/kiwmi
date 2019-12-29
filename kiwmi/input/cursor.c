/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "input/cursor.h"

#include <stdlib.h>

#include <wayland-server.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/util/log.h>

#include "desktop/desktop.h"
#include "desktop/view.h"
#include "server.h"

static void
process_cursor_motion(struct kiwmi_server *server, uint32_t time)
{
    struct kiwmi_desktop *desktop = &server->desktop;
    struct kiwmi_input *input     = &server->input;
    struct kiwmi_cursor *cursor   = input->cursor;
    struct wlr_seat *seat         = input->seat;

    struct wlr_surface *surface = NULL;
    double sx;
    double sy;

    struct kiwmi_view *view = view_at(
        desktop, cursor->cursor->x, cursor->cursor->y, &surface, &sx, &sy);

    if (!view) {
        wlr_xcursor_manager_set_cursor_image(
            cursor->xcursor_manager, "left_ptr", cursor->cursor);
    }

    if (surface) {
        bool focus_changed = surface != seat->pointer_state.focused_surface;

        wlr_seat_pointer_notify_enter(seat, surface, sx, sy);

        if (!focus_changed) {
            wlr_seat_pointer_notify_motion(seat, time, sx, sy);
        }
    } else {
        wlr_seat_pointer_clear_focus(seat);
    }
}

static void
cursor_motion_notify(struct wl_listener *listener, void *data)
{
    struct kiwmi_cursor *cursor =
        wl_container_of(listener, cursor, cursor_motion);
    struct kiwmi_server *server            = cursor->server;
    struct wlr_event_pointer_motion *event = data;

    wlr_cursor_move(
        cursor->cursor, event->device, event->delta_x, event->delta_y);

    process_cursor_motion(server, event->time_msec);
}

static void
cursor_motion_absolute_notify(struct wl_listener *listener, void *data)
{
    struct kiwmi_cursor *cursor =
        wl_container_of(listener, cursor, cursor_motion_absolute);
    struct kiwmi_server *server                     = cursor->server;
    struct wlr_event_pointer_motion_absolute *event = data;

    wlr_cursor_warp_absolute(cursor->cursor, event->device, event->x, event->y);

    process_cursor_motion(server, event->time_msec);
}

static void
cursor_button_notify(struct wl_listener *listener, void *data)
{
    struct kiwmi_cursor *cursor =
        wl_container_of(listener, cursor, cursor_button);
    struct kiwmi_server *server            = cursor->server;
    struct kiwmi_input *input              = &server->input;
    struct wlr_event_pointer_button *event = data;

    wlr_seat_pointer_notify_button(
        input->seat, event->time_msec, event->button, event->state);

    double sx;
    double sy;
    struct wlr_surface *surface;

    struct kiwmi_view *view = view_at(
        &server->desktop,
        cursor->cursor->x,
        cursor->cursor->y,
        &surface,
        &sx,
        &sy);

    if (event->state == WLR_BUTTON_PRESSED) {
        focus_view(view);
    }
}

static void
request_set_cursor_notify(struct wl_listener *listener, void *data)
{
    struct kiwmi_cursor *cursor =
        wl_container_of(listener, cursor, request_set_cursor);
    struct wlr_seat_pointer_request_set_cursor_event *event = data;

    struct wlr_surface *focused_surface =
        event->seat_client->seat->pointer_state.focused_surface;
    struct wl_client *focused_client = NULL;

    if (focused_surface && focused_surface->resource) {
        focused_client = wl_resource_get_client(focused_surface->resource);
    }

    if (event->seat_client->client != focused_client) {
        wlr_log(
            WLR_DEBUG, "Ignoring request to set cursor on unfocused client");
        return;
    }

    wlr_cursor_set_surface(
        cursor->cursor, event->surface, event->hotspot_x, event->hotspot_y);
}

struct kiwmi_cursor *
cursor_create(
    struct kiwmi_server *server,
    struct wlr_output_layout *output_layout)
{
    wlr_log(WLR_DEBUG, "Creating cursor");

    struct kiwmi_cursor *cursor = malloc(sizeof(*cursor));
    if (!cursor) {
        wlr_log(WLR_ERROR, "Failed to allocate kiwmi_cursor");
        return NULL;
    }

    cursor->server = server;

    cursor->cursor = wlr_cursor_create();
    if (!cursor->cursor) {
        wlr_log(WLR_ERROR, "Failed to create cursor");
        free(cursor);
        return NULL;
    }

    wlr_cursor_attach_output_layout(cursor->cursor, output_layout);

    cursor->xcursor_manager = wlr_xcursor_manager_create(NULL, 24);

    cursor->cursor_motion.notify = cursor_motion_notify;
    wl_signal_add(&cursor->cursor->events.motion, &cursor->cursor_motion);

    cursor->cursor_motion_absolute.notify = cursor_motion_absolute_notify;
    wl_signal_add(
        &cursor->cursor->events.motion_absolute,
        &cursor->cursor_motion_absolute);

    cursor->cursor_button.notify = cursor_button_notify;
    wl_signal_add(&cursor->cursor->events.button, &cursor->cursor_button);

    cursor->request_set_cursor.notify = request_set_cursor_notify;
    wl_signal_add(
        &server->input.seat->events.request_set_cursor,
        &cursor->request_set_cursor);

    return cursor;
}
