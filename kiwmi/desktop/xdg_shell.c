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
#include <wlr/types/wlr_xdg_decoration_v1.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/util/edges.h>
#include <wlr/util/log.h>

#include "desktop/desktop.h"
#include "desktop/output.h"
#include "desktop/view.h"
#include "input/cursor.h"
#include "input/input.h"
#include "input/seat.h"
#include "server.h"

static struct kiwmi_view_child *view_child_popup_create(
    struct kiwmi_view_child *parent,
    struct kiwmi_view *view,
    struct wlr_xdg_popup *wlr_popup);

static void
popup_new_popup_notify(struct wl_listener *listener, void *data)
{
    struct kiwmi_view_child *popup =
        wl_container_of(listener, popup, new_popup);
    struct wlr_xdg_popup *wlr_popup = data;
    view_child_popup_create(popup, popup->view, wlr_popup);
}

static void
popup_extension_destroy_notify(struct wl_listener *listener, void *UNUSED(data))
{
    struct kiwmi_view_child *popup =
        wl_container_of(listener, popup, extension_destroy);
    view_child_destroy(popup);
}

static void
popup_unconstrain(struct kiwmi_view_child *popup)
{
    if (popup->type != KIWMI_VIEW_CHILD_XDG_POPUP) {
        wlr_log(WLR_ERROR, "Expected an xdg_popup kiwmi_view_child");
        return;
    }

    struct kiwmi_view *view = popup->view;

    // Prefer output at view center
    struct wlr_output *output = wlr_output_layout_output_at(
        view->desktop->output_layout,
        view->x + view->geom.width / 2,
        view->y + view->geom.height / 2);

    if (!output) {
        // Retry with view top-left corner (if its center is off-screen)
        output = wlr_output_layout_output_at(
            view->desktop->output_layout, view->x, view->y);
    }

    if (!output) {
        wlr_log(
            WLR_ERROR, "View's output not found, popups may end up invisible");
        return;
    }

    double view_ox = view->x;
    double view_oy = view->y;
    wlr_output_layout_output_coords(
        view->desktop->output_layout, output, &view_ox, &view_oy);

    int output_width;
    int output_height;
    wlr_output_effective_resolution(output, &output_width, &output_height);

    // relative to the view
    struct wlr_box output_box = {
        .x      = -view_ox,
        .y      = -view_oy,
        .width  = output_width,
        .height = output_height,
    };

    wlr_xdg_popup_unconstrain_from_box(popup->wlr_xdg_popup, &output_box);
}

static void
popup_reconfigure(struct kiwmi_view_child *popup)
{
    if (popup->type != KIWMI_VIEW_CHILD_XDG_POPUP) {
        wlr_log(WLR_ERROR, "Expected an xdg_popup view_child");
        return;
    }

    popup_unconstrain(popup);

    struct kiwmi_view_child *subchild;
    wl_list_for_each (subchild, &popup->children, link) {
        if (subchild->impl && subchild->impl->reconfigure) {
            subchild->impl->reconfigure(subchild);
        }
    }
}

static const struct kiwmi_view_child_impl xdg_popup_view_child_impl = {
    .reconfigure = popup_reconfigure,
};

static struct kiwmi_view_child *
view_child_popup_create(
    struct kiwmi_view_child *parent,
    struct kiwmi_view *view,
    struct wlr_xdg_popup *wlr_popup)
{
    struct kiwmi_view_child *child = view_child_create(
        parent,
        view,
        wlr_popup->base->surface,
        KIWMI_VIEW_CHILD_XDG_POPUP,
        &xdg_popup_view_child_impl);
    if (!child) {
        return NULL;
    }

    child->wlr_xdg_popup = wlr_popup;
    child->mapped        = wlr_popup->base->mapped;

    if (view_child_is_mapped(child)) {
        view_child_damage(child);
    }

    wl_signal_add(&wlr_popup->base->events.map, &child->map);
    wl_signal_add(&wlr_popup->base->events.unmap, &child->unmap);

    child->new_popup.notify = popup_new_popup_notify;
    wl_signal_add(&wlr_popup->base->events.new_popup, &child->new_popup);

    child->extension_destroy.notify = popup_extension_destroy_notify;
    wl_signal_add(&wlr_popup->base->events.destroy, &child->extension_destroy);

    popup_unconstrain(child);

    return child;
}

static void
xdg_surface_new_popup_notify(struct wl_listener *listener, void *data)
{
    struct kiwmi_view *view = wl_container_of(listener, view, new_popup);
    struct wlr_xdg_popup *wlr_popup = data;
    view_child_popup_create(NULL, view, wlr_popup);
}

static void
xdg_surface_new_subsurface_notify(struct wl_listener *listener, void *data)
{
    struct kiwmi_view *view = wl_container_of(listener, view, new_subsurface);
    struct wlr_subsurface *subsurface = data;
    view_child_subsurface_create(NULL, view, subsurface);
}

static void
xdg_surface_map_notify(struct wl_listener *listener, void *UNUSED(data))
{
    struct kiwmi_view *view = wl_container_of(listener, view, map);
    view->mapped            = true;

    struct kiwmi_output *output;
    wl_list_for_each (output, &view->desktop->outputs, link) {
        output_damage(output);
    }

    wl_signal_emit(&view->desktop->events.view_map, view);
}

static void
xdg_surface_unmap_notify(struct wl_listener *listener, void *UNUSED(data))
{
    struct kiwmi_view *view = wl_container_of(listener, view, unmap);

    if (view->mapped) {
        view->mapped = false;

        struct kiwmi_output *output;
        wl_list_for_each (output, &view->desktop->outputs, link) {
            output_damage(output);
        }

        wl_signal_emit(&view->events.unmap, view);
    }
}

static void
xdg_surface_commit_notify(struct wl_listener *listener, void *UNUSED(data))
{
    struct kiwmi_view *view = wl_container_of(listener, view, commit);

    struct kiwmi_desktop *desktop = view->desktop;
    struct kiwmi_server *server   = wl_container_of(desktop, server, desktop);
    struct kiwmi_cursor *cursor   = server->input.cursor;
    cursor_refresh_focus(cursor, NULL, NULL, NULL);

    if (pixman_region32_not_empty(&view->wlr_surface->buffer_damage)) {
        struct kiwmi_output *output;
        wl_list_for_each (output, &desktop->outputs, link) {
            output_damage(output);
        }
    }

    wlr_xdg_surface_get_geometry(view->xdg_surface, &view->geom);
}

static void
xdg_surface_destroy_notify(struct wl_listener *listener, void *UNUSED(data))
{
    struct kiwmi_view *view       = wl_container_of(listener, view, destroy);
    struct kiwmi_desktop *desktop = view->desktop;
    struct kiwmi_server *server   = wl_container_of(desktop, server, desktop);
    struct kiwmi_seat *seat       = server->input.seat;

    if (seat->focused_view == view) {
        seat->focused_view = NULL;
    }
    cursor_refresh_focus(server->input.cursor, NULL, NULL, NULL);

    struct kiwmi_view_child *child, *tmpchild;
    wl_list_for_each_safe (child, tmpchild, &view->children, link) {
        child->mapped = false;
        view_child_destroy(child);
    }

    if (view->decoration) {
        view->decoration->view = NULL;
    }

    wl_list_remove(&view->link);
    wl_list_remove(&view->children);
    wl_list_remove(&view->map.link);
    wl_list_remove(&view->unmap.link);
    wl_list_remove(&view->commit.link);
    wl_list_remove(&view->destroy.link);
    wl_list_remove(&view->new_popup.link);
    wl_list_remove(&view->new_subsurface.link);
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
        wlr_xdg_toplevel_send_close(surface);
    }
}

static void
xdg_shell_view_for_each_surface(
    struct kiwmi_view *view,
    wlr_surface_iterator_func_t callback,
    void *user_data)
{
    wlr_xdg_surface_for_each_surface(view->xdg_surface, callback, user_data);
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
    wlr_xdg_toplevel_set_activated(view->xdg_surface, activated);
}

static void
xdg_shell_view_set_size(
    struct kiwmi_view *view,
    uint32_t width,
    uint32_t height)
{
    wlr_xdg_toplevel_set_size(view->xdg_surface, width, height);
}

static void
xdg_shell_view_set_tiled(struct kiwmi_view *view, enum wlr_edges edges)
{
    wlr_xdg_toplevel_set_tiled(view->xdg_surface, edges);
}

struct wlr_surface *
xdg_shell_view_surface_at(
    struct kiwmi_view *view,
    double sx,
    double sy,
    double *sub_x,
    double *sub_y)
{
    return wlr_xdg_surface_surface_at(view->xdg_surface, sx, sy, sub_x, sub_y);
}

static const struct kiwmi_view_impl xdg_shell_view_impl = {
    .close            = xdg_shell_view_close,
    .for_each_surface = xdg_shell_view_for_each_surface,
    .get_pid          = xdg_shell_view_get_pid,
    .get_string_prop  = xdg_shell_view_get_string_prop,
    .set_activated    = xdg_shell_view_set_activated,
    .set_size         = xdg_shell_view_set_size,
    .set_tiled        = xdg_shell_view_set_tiled,
    .surface_at       = xdg_shell_view_surface_at,
};

void
xdg_shell_new_surface_notify(struct wl_listener *listener, void *data)
{
    struct wlr_xdg_surface *xdg_surface = data;
    struct kiwmi_desktop *desktop =
        wl_container_of(listener, desktop, xdg_shell_new_surface);

    if (xdg_surface->role == WLR_XDG_SURFACE_ROLE_POPUP) {
        wlr_log(WLR_DEBUG, "New xdg_shell popup");
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

    view->map.notify = xdg_surface_map_notify;
    wl_signal_add(&xdg_surface->events.map, &view->map);

    view->unmap.notify = xdg_surface_unmap_notify;
    wl_signal_add(&xdg_surface->events.unmap, &view->unmap);

    view->commit.notify = xdg_surface_commit_notify;
    wl_signal_add(&xdg_surface->surface->events.commit, &view->commit);

    view->destroy.notify = xdg_surface_destroy_notify;
    wl_signal_add(&xdg_surface->events.destroy, &view->destroy);

    view->new_popup.notify = xdg_surface_new_popup_notify;
    wl_signal_add(&xdg_surface->events.new_popup, &view->new_popup);

    view->new_subsurface.notify = xdg_surface_new_subsurface_notify;
    wl_signal_add(
        &xdg_surface->surface->events.new_subsurface, &view->new_subsurface);

    view->request_move.notify = xdg_toplevel_request_move_notify;
    wl_signal_add(
        &xdg_surface->toplevel->events.request_move, &view->request_move);

    view->request_resize.notify = xdg_toplevel_request_resize_notify;
    wl_signal_add(
        &xdg_surface->toplevel->events.request_resize, &view->request_resize);

    view_init_subsurfaces(NULL, view);

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
