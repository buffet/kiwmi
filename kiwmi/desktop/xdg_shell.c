/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "desktop/xdg_shell.h"

#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/util/edges.h>
#include <wlr/util/log.h>

#include "desktop/desktop.h"
#include "desktop/view.h"
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
    view->mapped            = false;

    wl_signal_emit(&view->events.unmap, view);
}

static void
xdg_surface_destroy_notify(struct wl_listener *listener, void *UNUSED(data))
{
    struct kiwmi_view *view       = wl_container_of(listener, view, destroy);
    struct kiwmi_desktop *desktop = view->desktop;

    if (desktop->focused_view == view) {
        desktop->focused_view = NULL;
    }

    wl_list_remove(&view->link);
    wl_list_remove(&view->map.link);
    wl_list_remove(&view->unmap.link);
    wl_list_remove(&view->destroy.link);

    free(view);
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
    wlr_surface_iterator_func_t iterator,
    void *user_data)
{
    wlr_xdg_surface_for_each_surface(view->xdg_surface, iterator, user_data);
}

static void
xdg_shell_view_set_activated(struct kiwmi_view *view, bool activated)
{
    wlr_xdg_toplevel_set_activated(view->xdg_surface, activated);
}

static void
xdg_shell_view_set_size(struct kiwmi_view *view, uint32_t width, uint32_t height)
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

    view->xdg_surface = xdg_surface;
    view->wlr_surface = xdg_surface->surface;

    view->map.notify = xdg_surface_map_notify;
    wl_signal_add(&xdg_surface->events.map, &view->map);

    view->unmap.notify = xdg_surface_unmap_notify;
    wl_signal_add(&xdg_surface->events.unmap, &view->unmap);

    view->destroy.notify = xdg_surface_destroy_notify;
    wl_signal_add(&xdg_surface->events.destroy, &view->destroy);

    wl_list_insert(&desktop->views, &view->link);
}
