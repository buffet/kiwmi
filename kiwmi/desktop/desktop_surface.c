/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "desktop/desktop_surface.h"

#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_layer_shell_v1.h>
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/types/wlr_xdg_shell.h>

#include "desktop/desktop.h"
#include "desktop/layer_shell.h"
#include "desktop/output.h"
#include "desktop/popup.h"
#include "desktop/view.h"

static struct kiwmi_desktop_surface *
desktop_surface_from_wlr_surface(struct wlr_surface *surface)
{
    struct wlr_subsurface *subsurface;
    while (wlr_surface_is_subsurface(surface)) {
        subsurface = wlr_subsurface_from_wlr_surface(surface);
        surface    = subsurface->parent;
    }

    if (wlr_surface_is_xdg_surface(surface)) {
        struct wlr_xdg_surface *xdg_surface =
            wlr_xdg_surface_from_wlr_surface(surface);
        switch (xdg_surface->role) {
        case WLR_XDG_SURFACE_ROLE_TOPLEVEL:;
            struct kiwmi_view *view = xdg_surface->data;
            return &view->desktop_surface;
            break;
        case WLR_XDG_SURFACE_ROLE_POPUP:
            return popup_get_desktop_surface(xdg_surface->popup);
            break;
        default:
            return NULL;
            break;
        }
    } else if (wlr_surface_is_layer_surface(surface)) {
        struct wlr_layer_surface_v1 *layer_surface =
            wlr_layer_surface_v1_from_wlr_surface(surface);
        struct kiwmi_layer *layer = layer_surface->data;
        return &layer->desktop_surface;
    }

    return NULL;
}

struct kiwmi_desktop_surface *
desktop_surface_at(struct kiwmi_desktop *desktop, double lx, double ly)
{
    double sx, sy;
    struct wlr_scene_node *node_at =
        wlr_scene_node_at(&desktop->scene->tree.node, lx, ly, &sx, &sy);

    if (!node_at || node_at->type != WLR_SCENE_NODE_BUFFER) {
        return NULL;
    }

    struct wlr_scene_buffer *scene_buffer = wlr_scene_buffer_from_node(node_at);
    struct wlr_scene_surface *scene_surface =
        wlr_scene_surface_from_buffer(scene_buffer);

    if (!scene_surface) {
        return NULL;
    }

    return desktop_surface_from_wlr_surface(scene_surface->surface);
}

struct kiwmi_output *
desktop_surface_get_output(struct kiwmi_desktop_surface *desktop_surface)
{
    if (desktop_surface->impl && desktop_surface->impl->get_output) {
        return desktop_surface->impl->get_output(desktop_surface);
    } else {
        return NULL;
    }
}

void
desktop_surface_get_pos(
    struct kiwmi_desktop_surface *desktop_surface,
    int *lx,
    int *ly)
{
    wlr_scene_node_coords(&desktop_surface->tree->node, lx, ly);
}
