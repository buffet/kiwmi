/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "desktop/view.h"

#include <wlr/types/wlr_cursor.h>
#include <wlr/util/log.h>

#include "desktop/output.h"
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

void
view_for_each_surface(
    struct kiwmi_view *view,
    wlr_surface_iterator_func_t callback,
    void *user_data)
{
    if (view->impl->for_each_surface) {
        view->impl->for_each_surface(view, callback, user_data);
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

        struct kiwmi_view_child *child;
        wl_list_for_each (child, &view->children, link) {
            if (child->impl && child->impl->reconfigure) {
                child->impl->reconfigure(child);
            }
        }

        struct kiwmi_output *output;
        wl_list_for_each (output, &view->desktop->outputs, link) {
            output_damage(output);
        }
    }
}

void
view_set_pos(struct kiwmi_view *view, uint32_t x, uint32_t y)
{
    view->x = x;
    view->y = y;

    struct kiwmi_view_child *child;
    wl_list_for_each (child, &view->children, link) {
        if (child->impl && child->impl->reconfigure) {
            child->impl->reconfigure(child);
        }
    }

    struct kiwmi_desktop *desktop = view->desktop;
    struct kiwmi_server *server   = wl_container_of(desktop, server, desktop);
    struct kiwmi_cursor *cursor   = server->input.cursor;
    cursor_refresh_focus(cursor, NULL, NULL, NULL);

    struct kiwmi_output *output;
    wl_list_for_each (output, &desktop->outputs, link) {
        output_damage(output);
    }
}

void
view_set_tiled(struct kiwmi_view *view, enum wlr_edges edges)
{
    if (view->impl->set_tiled) {
        view->impl->set_tiled(view, edges);
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

static bool
surface_at(
    struct kiwmi_view *view,
    struct wlr_surface **surface,
    double lx,
    double ly,
    double *sx,
    double *sy)
{
    double view_sx = lx - view->x + view->geom.x;
    double view_sy = ly - view->y + view->geom.y;

    double _sx;
    double _sy;
    struct wlr_surface *_surface =
        view_surface_at(view, view_sx, view_sy, &_sx, &_sy);

    if (_surface) {
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
    wl_list_for_each (view, &desktop->views, link) {
        if (view->hidden || !view->mapped) {
            continue;
        }

        if (surface_at(view, surface, lx, ly, sx, sy)) {
            return view;
        }
    }

    return NULL;
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

    if (mode == KIWMI_CURSOR_MOVE) {
        cursor->grabbed.orig_x = cursor->cursor->x - view->x;
        cursor->grabbed.orig_y = cursor->cursor->y - view->y;
    } else {
        cursor->grabbed.orig_x       = cursor->cursor->x;
        cursor->grabbed.orig_y       = cursor->cursor->y;
        cursor->grabbed.resize_edges = edges;
    }

    cursor->grabbed.orig_geom.x      = view->x;
    cursor->grabbed.orig_geom.y      = view->y;
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

/**
 * Creates a kiwmi_view_child for each subsurface of either the 'child' or the
 * 'view'. 'child' can be NULL, 'view' should never be.
 */
void
view_init_subsurfaces(struct kiwmi_view_child *child, struct kiwmi_view *view)
{
    struct wlr_surface *surface =
        child ? child->wlr_surface : view->wlr_surface;
    if (!surface) {
        wlr_log(WLR_ERROR, "Attempting to init_subsurfaces without a surface");
        return;
    }

    struct wlr_subsurface *subsurface;
    wl_list_for_each (
        subsurface, &surface->current.subsurfaces_below, current.link) {
        view_child_subsurface_create(child, view, subsurface);
    }
    wl_list_for_each (
        subsurface, &surface->current.subsurfaces_above, current.link) {
        view_child_subsurface_create(child, view, subsurface);
    }
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

    view->desktop    = desktop;
    view->type       = type;
    view->impl       = impl;
    view->mapped     = false;
    view->hidden     = true;
    view->decoration = NULL;

    view->x = 0;
    view->y = 0;

    wl_list_init(&view->children);

    wl_signal_init(&view->events.unmap);
    wl_signal_init(&view->events.request_move);
    wl_signal_init(&view->events.request_resize);
    wl_signal_init(&view->events.post_render);
    wl_signal_init(&view->events.pre_render);

    return view;
}

bool
view_child_is_mapped(struct kiwmi_view_child *child)
{
    if (!child->mapped) {
        return false;
    }

    struct kiwmi_view_child *parent = child->parent;
    while (parent) {
        if (!parent->mapped) {
            return false;
        }
        parent = parent->parent;
    }

    return child->view->mapped;
}

void
view_child_damage(struct kiwmi_view_child *child)
{
    // Note for later: this is supposed to damage the child and all subchildren
    struct kiwmi_output *output;
    wl_list_for_each (output, &child->view->desktop->outputs, link) {
        output_damage(output);
    }
}

void
view_child_destroy(struct kiwmi_view_child *child)
{
    bool visible = view_child_is_mapped(child) && !child->view->hidden;
    if (visible) {
        view_child_damage(child);
    }

    wl_list_remove(&child->link);
    child->parent = NULL;

    struct kiwmi_view_child *subchild, *tmpchild;
    wl_list_for_each_safe (subchild, tmpchild, &child->children, link) {
        subchild->mapped = false;
        view_child_destroy(subchild);
    }

    wl_list_remove(&child->commit.link);
    wl_list_remove(&child->map.link);
    wl_list_remove(&child->unmap.link);
    wl_list_remove(&child->new_popup.link);
    wl_list_remove(&child->new_subsurface.link);
    wl_list_remove(&child->surface_destroy.link);
    wl_list_remove(&child->extension_destroy.link);

    free(child);
}

static void
view_child_subsurface_extension_destroy_notify(
    struct wl_listener *listener,
    void *UNUSED(data))
{
    struct kiwmi_view_child *child =
        wl_container_of(listener, child, extension_destroy);
    view_child_destroy(child);
}

static void
view_child_surface_destroy_notify(
    struct wl_listener *listener,
    void *UNUSED(data))
{
    struct kiwmi_view_child *child =
        wl_container_of(listener, child, surface_destroy);
    view_child_destroy(child);
}

static void
view_child_commit_notify(struct wl_listener *listener, void *UNUSED(data))
{
    struct kiwmi_view_child *child = wl_container_of(listener, child, commit);
    if (view_child_is_mapped(child)) {
        view_child_damage(child);
    }
}

static void
view_child_new_subsurface_notify(struct wl_listener *listener, void *data)
{
    struct kiwmi_view_child *child =
        wl_container_of(listener, child, new_subsurface);
    struct wlr_subsurface *subsurface = data;
    view_child_subsurface_create(child, child->view, subsurface);
}

static void
view_child_map_notify(struct wl_listener *listener, void *UNUSED(data))
{
    struct kiwmi_view_child *child = wl_container_of(listener, child, map);
    child->mapped                  = true;
    if (view_child_is_mapped(child)) {
        view_child_damage(child);
    }
}

static void
view_child_unmap_notify(struct wl_listener *listener, void *UNUSED(data))
{
    struct kiwmi_view_child *child = wl_container_of(listener, child, unmap);
    if (view_child_is_mapped(child)) {
        view_child_damage(child);
    }
    child->mapped = false;
}

struct kiwmi_view_child *
view_child_create(
    struct kiwmi_view_child *parent,
    struct kiwmi_view *view,
    struct wlr_surface *wlr_surface,
    enum kiwmi_view_child_type type,
    const struct kiwmi_view_child_impl *impl)
{
    struct kiwmi_view_child *child = calloc(1, sizeof(*child));
    if (!child) {
        wlr_log(WLR_ERROR, "Failed to allocate view_child");
        return NULL;
    }

    child->type        = type;
    child->impl        = impl;
    child->view        = view;
    child->wlr_surface = wlr_surface;
    child->mapped      = false;

    if (parent) {
        child->parent = parent;
        wl_list_insert(&parent->children, &child->link);
    } else {
        wl_list_insert(&view->children, &child->link);
    }

    wl_list_init(&child->children);

    child->commit.notify = view_child_commit_notify;
    wl_signal_add(&wlr_surface->events.commit, &child->commit);

    child->map.notify   = view_child_map_notify;
    child->unmap.notify = view_child_unmap_notify;

    // wlr_surface doesn't have these events, but its extensions usually do
    wl_list_init(&child->map.link);
    wl_list_init(&child->unmap.link);

    child->new_subsurface.notify = view_child_new_subsurface_notify;
    wl_signal_add(&wlr_surface->events.new_subsurface, &child->new_subsurface);

    child->surface_destroy.notify = view_child_surface_destroy_notify;
    wl_signal_add(&wlr_surface->events.destroy, &child->surface_destroy);

    // Possibly unused
    wl_list_init(&child->new_popup.link);
    wl_list_init(&child->extension_destroy.link);

    view_init_subsurfaces(child, child->view);

    return child;
}

struct kiwmi_view_child *
view_child_subsurface_create(
    struct kiwmi_view_child *parent,
    struct kiwmi_view *view,
    struct wlr_subsurface *subsurface)
{
    struct kiwmi_view_child *child = view_child_create(
        parent, view, subsurface->surface, KIWMI_VIEW_CHILD_SUBSURFACE, NULL);
    if (!child) {
        return NULL;
    }

    child->wlr_subsurface = subsurface;
    child->mapped         = subsurface->mapped;

    if (view_child_is_mapped(child)) {
        view_child_damage(child);
    }

    wl_signal_add(&subsurface->events.map, &child->map);
    wl_signal_add(&subsurface->events.unmap, &child->unmap);

    child->extension_destroy.notify =
        view_child_subsurface_extension_destroy_notify;
    wl_signal_add(&subsurface->events.destroy, &child->extension_destroy);

    return child;
}
