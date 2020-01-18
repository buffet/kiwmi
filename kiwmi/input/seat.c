/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "input/seat.h"

#include <stdlib.h>

#include <wayland-server.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/util/log.h>

#include "input/cursor.h"
#include "server.h"

static void
request_set_cursor_notify(struct wl_listener *listener, void *data)
{
    struct kiwmi_seat *seat =
        wl_container_of(listener, seat, request_set_cursor);
    struct kiwmi_cursor *cursor = seat->input->cursor;
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

struct kiwmi_seat *
seat_create(struct kiwmi_input *input)
{
    struct kiwmi_server *server = wl_container_of(input, server, input);

    struct kiwmi_seat *seat = malloc(sizeof(*seat));
    if (!seat) {
        wlr_log(WLR_ERROR, "Failed to allocate kiwmi_seat");
        return NULL;
    }

    seat->input = input;
    seat->seat  = wlr_seat_create(server->wl_display, "seat-0");

    seat->request_set_cursor.notify = request_set_cursor_notify;
    wl_signal_add(
        &seat->seat->events.request_set_cursor, &seat->request_set_cursor);

    return seat;
}

void
seat_destroy(struct kiwmi_seat *seat)
{
    wl_list_remove(&seat->request_set_cursor.link);

    free(seat);
}
