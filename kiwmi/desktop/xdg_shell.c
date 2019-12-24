/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "desktop/xdg_shell.h"

#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/util/log.h>

#include "desktop/desktop.h"
#include "desktop/view.h"
#include "server.h"

static void
xdg_surface_map_notify(struct wl_listener *listener, void *UNUSED(data))
{
    struct kiwmi_view *view = wl_container_of(listener, view, map);
    view->mapped            = true;
    focus_view(view, view->xdg_surface->surface);
}

static void
xdg_surface_unmap_notify(struct wl_listener *listener, void *UNUSED(data))
{
    struct kiwmi_view *view = wl_container_of(listener, view, map);
    view->mapped            = false;
}

static void
xdg_surface_destroy_notify(struct wl_listener *listener, void *UNUSED(data))
{
    struct kiwmi_view *view = wl_container_of(listener, view, destroy);

    wl_list_remove(&view->link);
    wl_list_remove(&view->map.link);
    wl_list_remove(&view->unmap.link);
    wl_list_remove(&view->destroy.link);

    free(view);
}

static void
xdg_shell_view_for_each_surface(
    struct kiwmi_view *view,
    wlr_surface_iterator_func_t iterator,
    void *user_data)
{
    wlr_xdg_surface_for_each_surface(view->xdg_surface, iterator, user_data);
}

static const struct kiwmi_view_impl xdg_shell_view_impl = {
    .for_each_surface = xdg_shell_view_for_each_surface,
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

    struct kiwmi_view *view = malloc(sizeof(*view));
    if (!view) {
        wlr_log(WLR_ERROR, "Failed to allocate view");
        return;
    }

    view->desktop     = desktop;
    view->type        = KIWMI_VIEW_XDG_SHELL;
    view->impl        = &xdg_shell_view_impl;
    view->xdg_surface = xdg_surface;
    view->mapped      = false;

    view->map.notify = xdg_surface_map_notify;
    wl_signal_add(&xdg_surface->events.map, &view->map);

    view->unmap.notify = xdg_surface_unmap_notify;
    wl_signal_add(&xdg_surface->events.unmap, &view->unmap);

    view->destroy.notify = xdg_surface_destroy_notify;
    wl_signal_add(&xdg_surface->events.destroy, &view->destroy);

    wl_list_insert(&desktop->views, &view->link);
}
