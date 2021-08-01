/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "input/seat.h"

#include <stdlib.h>

#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_layer_shell_v1.h>
#include <wlr/types/wlr_primary_selection.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/util/log.h>

#include "desktop/layer_shell.h"
#include "desktop/view.h"
#include "input/cursor.h"
#include "server.h"

void
seat_focus_surface(struct kiwmi_seat *seat, struct wlr_surface *wlr_surface)
{
    if (seat->focused_layer) {
        return;
    }

    struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(seat->seat);
    if (!keyboard) {
        wlr_seat_keyboard_enter(seat->seat, wlr_surface, NULL, 0, NULL);
    }

    wlr_seat_keyboard_enter(
        seat->seat,
        wlr_surface,
        keyboard->keycodes,
        keyboard->num_keycodes,
        &keyboard->modifiers);
}

void
seat_focus_view(struct kiwmi_seat *seat, struct kiwmi_view *view)
{
    if (!view) {
        seat_focus_surface(seat, NULL);
    }

    struct kiwmi_desktop *desktop = view->desktop;

    if (seat->focused_view) {
        view_set_activated(seat->focused_view, false);
    }

    // move view to front
    wl_list_remove(&view->link);
    wl_list_insert(&desktop->views, &view->link);

    seat->focused_view = view;
    view_set_activated(view, true);
    seat_focus_surface(seat, view->wlr_surface);
}

void
seat_focus_layer(struct kiwmi_seat *seat, struct kiwmi_layer *layer)
{
    if (!layer) {
        seat->focused_layer = NULL;

        if (seat->focused_view) {
            seat_focus_surface(seat, seat->focused_view->wlr_surface);
        }

        return;
    }

    if (seat->focused_layer == layer) {
        return;
    }

    seat->focused_layer = NULL;

    seat_focus_surface(seat, layer->layer_surface->surface);

    seat->focused_layer = layer;
}

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

static void
request_set_selection_notify(struct wl_listener *listener, void *data)
{
    struct kiwmi_seat *seat =
        wl_container_of(listener, seat, request_set_selection);
    struct wlr_seat_request_set_selection_event *event = data;
    wlr_seat_set_selection(seat->seat, event->source, event->serial);
}

static void
request_set_primary_selection_notify(struct wl_listener *listener, void *data)
{
    struct kiwmi_seat *seat =
        wl_container_of(listener, seat, request_set_primary_selection);
    struct wlr_seat_request_set_primary_selection_event *event = data;
    wlr_seat_set_primary_selection(seat->seat, event->source, event->serial);
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

    seat->focused_view  = NULL;
    seat->focused_layer = NULL;

    seat->request_set_cursor.notify = request_set_cursor_notify;
    wl_signal_add(
        &seat->seat->events.request_set_cursor, &seat->request_set_cursor);

    wl_signal_add(
        &seat->seat->events.request_set_selection,
        &seat->request_set_selection);
    seat->request_set_selection.notify = request_set_selection_notify;

    wl_signal_add(
        &seat->seat->events.request_set_primary_selection,
        &seat->request_set_primary_selection);
    seat->request_set_primary_selection.notify =
        request_set_primary_selection_notify;

    return seat;
}

void
seat_destroy(struct kiwmi_seat *seat)
{
    wl_list_remove(&seat->request_set_cursor.link);
    wl_list_remove(&seat->request_set_selection.link);
    wl_list_remove(&seat->request_set_primary_selection.link);

    free(seat);
}
