/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "desktop/view.h"

#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/util/log.h>

#include "desktop/output.h"
#include "desktop/stratum.h"
#include "input/cursor.h"
#include "input/seat.h"
#include "server.h"

void
view_close(struct kiwmi_view *view)
{
    if (view->impl->close) {
        view->impl->close(view);
    }
}

pid_t
view_get_pid(struct kiwmi_view *view)
{
    if (view->impl->get_pid) {
        return view->impl->get_pid(view);
    }

    return -1;
}

void
view_get_size(struct kiwmi_view *view, uint32_t *width, uint32_t *height)
{
    *width  = view->geom.width;
    *height = view->geom.height;
}

const char *
view_get_app_id(struct kiwmi_view *view)
{
    if (view->impl->get_string_prop) {
        return view->impl->get_string_prop(view, KIWMI_VIEW_PROP_APP_ID);
    }

    return NULL;
}

const char *
view_get_title(struct kiwmi_view *view)
{
    if (view->impl->get_string_prop) {
        return view->impl->get_string_prop(view, KIWMI_VIEW_PROP_TITLE);
    }

    return NULL;
}

void
view_set_activated(struct kiwmi_view *view, bool activated)
{
    if (view->impl->set_activated) {
        view->impl->set_activated(view, activated);
    }
}

void
view_set_size(struct kiwmi_view *view, uint32_t width, uint32_t height)
{
    if (view->impl->set_size) {
        view->impl->set_size(view, width, height);
    }
}

void
view_set_pos(struct kiwmi_view *view, uint32_t x, uint32_t y)
{
    wlr_scene_node_set_position(&view->desktop_surface.tree->node, x, y);
    wlr_scene_node_set_position(&view->desktop_surface.popups_tree->node, x, y);

    int lx, ly; // unused
    // If it is enabled (as well as all its parents)
    if (wlr_scene_node_coords(&view->desktop_surface.tree->node, &lx, &ly)) {
        struct kiwmi_server *server =
            wl_container_of(view->desktop, server, desktop);
        cursor_refresh_focus(server->input.cursor, NULL, NULL, NULL);
    }
}

void
view_set_tiled(struct kiwmi_view *view, enum wlr_edges edges)
{
    if (view->impl->set_tiled) {
        view->impl->set_tiled(view, edges);
    }
}

void
view_set_hidden(struct kiwmi_view *view, bool hidden)
{
    if (!view->mapped) {
        return;
    }

    wlr_scene_node_set_enabled(&view->desktop_surface.tree->node, !hidden);
    wlr_scene_node_set_enabled(
        &view->desktop_surface.popups_tree->node, !hidden);

    struct kiwmi_server *server =
        wl_container_of(view->desktop, server, desktop);
    struct kiwmi_seat *seat = server->input.seat;
    if (seat->focused_view == view) {
        seat->focused_view = NULL;
    }
}

struct kiwmi_view *
view_at(struct kiwmi_desktop *desktop, double lx, double ly)
{
    struct kiwmi_desktop_surface *desktop_surface =
        desktop_surface_at(desktop, lx, ly);

    if (!desktop_surface
        || desktop_surface->type != KIWMI_DESKTOP_SURFACE_VIEW) {
        return NULL;
    }

    struct kiwmi_view *view =
        wl_container_of(desktop_surface, view, desktop_surface);
    return view;
}

static void
view_begin_interactive(
    struct kiwmi_view *view,
    enum kiwmi_cursor_mode mode,
    uint32_t edges)
{
    struct kiwmi_desktop *desktop = view->desktop;
    struct kiwmi_server *server   = wl_container_of(desktop, server, desktop);
    struct kiwmi_cursor *cursor   = server->input.cursor;
    struct wlr_surface *focused_surface =
        server->input.seat->seat->pointer_state.focused_surface;
    struct wlr_surface *wlr_surface = view->wlr_surface;

    if (wlr_surface != focused_surface) {
        return;
    }

    uint32_t width;
    uint32_t height;
    view_get_size(view, &width, &height);

    cursor->cursor_mode  = mode;
    cursor->grabbed.view = view;

    int view_lx, view_ly;
    desktop_surface_get_pos(&view->desktop_surface, &view_lx, &view_ly);

    if (mode == KIWMI_CURSOR_MOVE) {
        cursor->grabbed.orig_x = cursor->cursor->x - view_lx;
        cursor->grabbed.orig_y = cursor->cursor->y - view_ly;
    } else {
        cursor->grabbed.orig_x       = cursor->cursor->x;
        cursor->grabbed.orig_y       = cursor->cursor->y;
        cursor->grabbed.resize_edges = edges;
    }

    cursor->grabbed.orig_geom.x      = view_lx;
    cursor->grabbed.orig_geom.y      = view_ly;
    cursor->grabbed.orig_geom.width  = width;
    cursor->grabbed.orig_geom.height = height;
}

void
view_move(struct kiwmi_view *view)
{
    view_begin_interactive(view, KIWMI_CURSOR_MOVE, 0);
}

void
view_resize(struct kiwmi_view *view, uint32_t edges)
{
    view_begin_interactive(view, KIWMI_CURSOR_RESIZE, edges);
}

static struct kiwmi_output *
view_desktop_surface_get_output(struct kiwmi_desktop_surface *desktop_surface)
{
    struct kiwmi_view *view =
        wl_container_of(desktop_surface, view, desktop_surface);

    int lx, ly;
    desktop_surface_get_pos(&view->desktop_surface, &lx, &ly);

    // Prefer view center
    struct wlr_output *output = wlr_output_layout_output_at(
        view->desktop->output_layout,
        lx + (float)view->geom.width / 2,
        ly + (float)view->geom.height / 2);
    if (output) {
        return (struct kiwmi_output *)output->data;
    }

    // Retry top-left corner
    output = wlr_output_layout_output_at(view->desktop->output_layout, lx, ly);
    if (output) {
        return (struct kiwmi_output *)output->data;
    }

    return NULL;
}

static const struct kiwmi_desktop_surface_impl view_desktop_surface_impl = {
    .get_output = view_desktop_surface_get_output,
};

struct kiwmi_view *
view_create(
    struct kiwmi_desktop *desktop,
    enum kiwmi_view_type type,
    const struct kiwmi_view_impl *impl)
{
    struct kiwmi_view *view = malloc(sizeof(*view));
    if (!view) {
        wlr_log(WLR_ERROR, "Failed to allocate view");
        return NULL;
    }

    view->desktop    = desktop;
    view->type       = type;
    view->impl       = impl;
    view->mapped     = false;
    view->decoration = NULL;

    view->desktop_surface.type = KIWMI_DESKTOP_SURFACE_VIEW;
    view->desktop_surface.impl = &view_desktop_surface_impl;

    wl_signal_init(&view->events.unmap);
    wl_signal_init(&view->events.request_move);
    wl_signal_init(&view->events.request_resize);
    wl_signal_init(&view->events.post_render);
    wl_signal_init(&view->events.pre_render);

    view->desktop_surface.tree =
        wlr_scene_tree_create(view->desktop->strata[KIWMI_STRATUM_NORMAL]);
    view->desktop_surface.popups_tree =
        wlr_scene_tree_create(view->desktop->strata[KIWMI_STRATUM_POPUPS]);

    view_set_hidden(view, true);
    wlr_scene_node_lower_to_bottom(&view->desktop_surface.tree->node);
    wlr_scene_node_lower_to_bottom(&view->desktop_surface.popups_tree->node);

    wlr_scene_node_set_position(&view->desktop_surface.tree->node, 0, 0);
    wlr_scene_node_set_position(&view->desktop_surface.popups_tree->node, 0, 0);

    return view;
}
