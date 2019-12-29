/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "desktop/view.h"

#include <wlr/util/log.h>

#include "desktop/output.h"
#include "server.h"

void
view_for_each_surface(
    struct kiwmi_view *view,
    wlr_surface_iterator_func_t iterator,
    void *user_data)
{
    if (view->impl->for_each_surface) {
        view->impl->for_each_surface(view, iterator, user_data);
    }
}

void
view_set_activated(struct kiwmi_view *view, bool activated)
{
    if (view->impl->set_activated) {
        view->impl->set_activated(view, activated);
    }
}

struct wlr_surface *
view_surface_at(
    struct kiwmi_view *view,
    double sx,
    double sy,
    double *sub_x,
    double *sub_y)
{
    if (view->impl->surface_at) {
        return view->impl->surface_at(view, sx, sy, sub_x, sub_y);
    }
    return NULL;
}

void
focus_view(struct kiwmi_view *view)
{
    if (!view) {
        return;
    }

    struct kiwmi_desktop *desktop = view->desktop;
    struct kiwmi_server *server   = wl_container_of(desktop, server, desktop);
    struct wlr_seat *seat         = server->input.seat;

    if (view == desktop->focused_view) {
        return;
    }

    if (desktop->focused_view) {
        view_set_activated(desktop->focused_view, false);
    }

    struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(seat);

    // move view to front
    wl_list_remove(&view->link);
    wl_list_insert(&desktop->views, &view->link);

    view_set_activated(view, true);
    wlr_seat_keyboard_notify_enter(
        seat,
        view->wlr_surface,
        keyboard->keycodes,
        keyboard->num_keycodes,
        &keyboard->modifiers);
}

static bool
surface_at(
    struct kiwmi_view *view,
    struct wlr_surface **surface,
    double lx,
    double ly,
    double *sx,
    double *sy)
{
    double view_sx = lx - view->x;
    double view_sy = ly - view->y;

    double _sx, _sy;
    struct wlr_surface *_surface = NULL;
    _surface = view_surface_at(view, view_sx, view_sy, &_sx, &_sy);

    if (_surface != NULL) {
        *sx      = _sx;
        *sy      = _sy;
        *surface = _surface;
        return true;
    }

    return false;
}

struct kiwmi_view *
view_at(
    struct kiwmi_desktop *desktop,
    double lx,
    double ly,
    struct wlr_surface **surface,
    double *sx,
    double *sy)
{
    struct kiwmi_view *view;
    wl_list_for_each(view, &desktop->views, link)
    {
        if (surface_at(view, surface, lx, ly, sx, sy)) {
            return view;
        }
    }

    return NULL;
}

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

    view->desktop = desktop;
    view->type    = type;
    view->impl    = impl;
    view->mapped  = false;

    view->x = 0;
    view->y = 0;

    return view;
}
