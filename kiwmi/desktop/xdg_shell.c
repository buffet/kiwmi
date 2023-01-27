/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "desktop/xdg_shell.h"

#include <unistd.h>

#include <pixman.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_xdg_decoration_v1.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/util/edges.h>
#include <wlr/util/log.h>

#include "desktop/desktop.h"
#include "desktop/output.h"
#include "desktop/popup.h"
#include "desktop/view.h"
#include "input/cursor.h"
#include "input/input.h"
#include "input/seat.h"
#include "server.h"

static void
xdg_surface_map_notify(struct wl_listener *listener, void *UNUSED(data))
{
    struct kiwmi_view *view = wl_container_of(listener, view, map);
    view->mapped            = true;

    wl_signal_emit(&view->desktop->events.view_map, view);
}

static void
xdg_surface_unmap_notify(struct wl_listener *listener, void *UNUSED(data))
{
    struct kiwmi_view *view = wl_container_of(listener, view, unmap);

    view->mapped = false;

    int lx, ly; // unused
    if (wlr_scene_node_coords(&view->desktop_surface.tree->node, &lx, &ly)) {
        wlr_scene_node_set_enabled(&view->desktop_surface.tree->node, false);
        wlr_scene_node_set_enabled(
            &view->desktop_surface.popups_tree->node, false);

        struct kiwmi_server *server =
            wl_container_of(view->desktop, server, desktop);
        cursor_refresh_focus(server->input.cursor, NULL, NULL, NULL);

        struct kiwmi_seat *seat = server->input.seat;
        if (seat->focused_view == view) {
            seat->focused_view = NULL;
        }
    }

    wl_signal_emit(&view->events.unmap, view);
}

static void
xdg_surface_commit_notify(struct wl_listener *listener, void *UNUSED(data))
{
    struct kiwmi_view *view = wl_container_of(listener, view, commit);

    struct wlr_box geom;
    wlr_xdg_surface_get_geometry(view->xdg_surface, &geom);

    if (memcmp(&view->geom, &geom, sizeof(geom)) != 0) {
        memcpy(&view->geom, &geom, sizeof(geom));

        struct kiwmi_desktop *desktop = view->desktop;
        struct kiwmi_server *server = wl_container_of(desktop, server, desktop);
        cursor_refresh_focus(server->input.cursor, NULL, NULL, NULL);
    }
}

static void
xdg_surface_destroy_notify(struct wl_listener *listener, void *UNUSED(data))
{
    struct kiwmi_view *view = wl_container_of(listener, view, destroy);

    wlr_scene_node_destroy(&view->desktop_surface.tree->node);
    wlr_scene_node_destroy(&view->desktop_surface.popups_tree->node);

    if (view->decoration) {
        view->decoration->view = NULL;
    }

    wl_list_remove(&view->link);
    wl_list_remove(&view->map.link);
    wl_list_remove(&view->unmap.link);
    wl_list_remove(&view->commit.link);
    wl_list_remove(&view->destroy.link);
    wl_list_remove(&view->request_move.link);
    wl_list_remove(&view->request_resize.link);

    wl_list_remove(&view->events.unmap.listener_list);

    free(view);
}

static void
xdg_toplevel_request_move_notify(
    struct wl_listener *listener,
    void *UNUSED(data))
{
    struct kiwmi_view *view = wl_container_of(listener, view, request_move);

    wl_signal_emit(&view->events.request_move, view);
}

static void
xdg_toplevel_request_resize_notify(struct wl_listener *listener, void *data)
{
    struct kiwmi_view *view = wl_container_of(listener, view, request_resize);
    struct wlr_xdg_toplevel_resize_event *event = data;

    struct kiwmi_request_resize_event new_event = {
        .view  = view,
        .edges = event->edges,
    };

    wl_signal_emit(&view->events.request_resize, &new_event);
}

static void
xdg_shell_view_close(struct kiwmi_view *view)
{
    struct wlr_xdg_surface *surface = view->xdg_surface;

    if (surface->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL && surface->toplevel) {
        wlr_xdg_toplevel_send_close(surface->toplevel);
    }
}

static pid_t
xdg_shell_view_get_pid(struct kiwmi_view *view)
{
    struct wl_client *client = view->xdg_surface->client->client;

    pid_t pid;
    wl_client_get_credentials(client, &pid, NULL, NULL);

    return pid;
}

static const char *
xdg_shell_view_get_string_prop(
    struct kiwmi_view *view,
    enum kiwmi_view_prop prop)
{
    switch (prop) {
    case KIWMI_VIEW_PROP_APP_ID:
        return view->xdg_surface->toplevel->app_id;
    case KIWMI_VIEW_PROP_TITLE:
        return view->xdg_surface->toplevel->title;
    default:
        return NULL;
    }
}

static void
xdg_shell_view_set_activated(struct kiwmi_view *view, bool activated)
{
    wlr_xdg_toplevel_set_activated(view->xdg_surface->toplevel, activated);
}

static void
xdg_shell_view_set_size(
    struct kiwmi_view *view,
    uint32_t width,
    uint32_t height)
{
    wlr_xdg_toplevel_set_size(view->xdg_surface->toplevel, width, height);
}

static void
xdg_shell_view_set_tiled(struct kiwmi_view *view, enum wlr_edges edges)
{
    wlr_xdg_toplevel_set_tiled(view->xdg_surface->toplevel, edges);
}

static const struct kiwmi_view_impl xdg_shell_view_impl = {
    .close           = xdg_shell_view_close,
    .get_pid         = xdg_shell_view_get_pid,
    .get_string_prop = xdg_shell_view_get_string_prop,
    .set_activated   = xdg_shell_view_set_activated,
    .set_size        = xdg_shell_view_set_size,
    .set_tiled       = xdg_shell_view_set_tiled,
};

void
xdg_shell_new_surface_notify(struct wl_listener *listener, void *data)
{
    struct wlr_xdg_surface *xdg_surface = data;
    struct kiwmi_desktop *desktop =
        wl_container_of(listener, desktop, xdg_shell_new_surface);

    if (xdg_surface->role == WLR_XDG_SURFACE_ROLE_POPUP) {
        wlr_log(WLR_DEBUG, "New xdg_shell popup");
        struct kiwmi_desktop_surface *desktop_surface =
            popup_get_desktop_surface(xdg_surface->popup);
        if (desktop_surface) {
            popup_attach(xdg_surface->popup, desktop_surface);

            struct kiwmi_server *server =
                wl_container_of(desktop, server, desktop);
            cursor_refresh_focus(server->input.cursor, NULL, NULL, NULL);
        }
        return;
    }

    wlr_log(
        WLR_DEBUG,
        "New xdg_shell toplevel title='%s' app_id='%s'",
        xdg_surface->toplevel->title,
        xdg_surface->toplevel->app_id);

    wlr_xdg_surface_ping(xdg_surface);

    struct kiwmi_view *view =
        view_create(desktop, KIWMI_VIEW_XDG_SHELL, &xdg_shell_view_impl);
    if (!view) {
        return;
    }

    xdg_surface->data = view;

    view->xdg_surface = xdg_surface;
    view->wlr_surface = xdg_surface->surface;

    view->desktop_surface.surface_tree =
        wlr_scene_xdg_surface_create(view->desktop_surface.tree, xdg_surface);

    view->map.notify = xdg_surface_map_notify;
    wl_signal_add(&xdg_surface->events.map, &view->map);

    view->unmap.notify = xdg_surface_unmap_notify;
    wl_signal_add(&xdg_surface->events.unmap, &view->unmap);

    view->commit.notify = xdg_surface_commit_notify;
    wl_signal_add(&xdg_surface->surface->events.commit, &view->commit);

    view->destroy.notify = xdg_surface_destroy_notify;
    wl_signal_add(&xdg_surface->events.destroy, &view->destroy);

    view->request_move.notify = xdg_toplevel_request_move_notify;
    wl_signal_add(
        &xdg_surface->toplevel->events.request_move, &view->request_move);

    view->request_resize.notify = xdg_toplevel_request_resize_notify;
    wl_signal_add(
        &xdg_surface->toplevel->events.request_resize, &view->request_resize);

    wlr_xdg_surface_get_geometry(view->xdg_surface, &view->geom);

    wl_list_insert(&desktop->views, &view->link);
}

static void
xdg_decoration_destroy_notify(struct wl_listener *listener, void *UNUSED(data))
{
    struct kiwmi_xdg_decoration *decoration =
        wl_container_of(listener, decoration, destroy);

    if (decoration->view) {
        decoration->view->decoration = NULL;
    }

    wl_list_remove(&decoration->destroy.link);
    wl_list_remove(&decoration->request_mode.link);

    free(decoration);
}

static void
xdg_decoration_request_mode_notify(
    struct wl_listener *listener,
    void *UNUSED(data))
{
    struct kiwmi_xdg_decoration *decoration =
        wl_container_of(listener, decoration, request_mode);

    enum wlr_xdg_toplevel_decoration_v1_mode mode =
        decoration->wlr_decoration->requested_mode;
    if (mode == WLR_XDG_TOPLEVEL_DECORATION_V1_MODE_NONE) {
        mode = WLR_XDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE;
    }

    wlr_xdg_toplevel_decoration_v1_set_mode(decoration->wlr_decoration, mode);
}

void
xdg_toplevel_new_decoration_notify(struct wl_listener *listener, void *data)
{
    struct kiwmi_desktop *desktop =
        wl_container_of(listener, desktop, xdg_toplevel_new_decoration);
    struct wlr_xdg_toplevel_decoration_v1 *wlr_decoration = data;

    wlr_log(WLR_DEBUG, "New xdg_toplevel decoration");

    struct kiwmi_view *view = wlr_decoration->surface->data;

    struct kiwmi_xdg_decoration *decoration = malloc(sizeof(*decoration));
    if (!decoration) {
        wlr_log(WLR_ERROR, "Failed to allocate xdg_decoration");
        return;
    }

    view->decoration = decoration;

    decoration->view           = view;
    decoration->wlr_decoration = wlr_decoration;

    decoration->destroy.notify = xdg_decoration_destroy_notify;
    wl_signal_add(&wlr_decoration->events.destroy, &decoration->destroy);

    decoration->request_mode.notify = xdg_decoration_request_mode_notify;
    wl_signal_add(
        &wlr_decoration->events.request_mode, &decoration->request_mode);
}
