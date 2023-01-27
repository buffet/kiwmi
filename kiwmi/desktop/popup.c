/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "desktop/popup.h"

#include <wlr/types/wlr_layer_shell_v1.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/util/log.h>

#include "desktop/desktop.h"
#include "desktop/desktop_surface.h"
#include "desktop/layer_shell.h"
#include "desktop/output.h"
#include "desktop/view.h"

struct kiwmi_desktop_surface *
popup_get_desktop_surface(struct wlr_xdg_popup *popup)
{
    struct wlr_surface *parent = popup->parent;
    while (parent) {
        if (wlr_surface_is_xdg_surface(parent)) {
            struct wlr_xdg_surface *xdg_surface =
                wlr_xdg_surface_from_wlr_surface(parent);
            switch (xdg_surface->role) {
            case WLR_XDG_SURFACE_ROLE_POPUP:
                parent = xdg_surface->popup->parent;
                break;
            case WLR_XDG_SURFACE_ROLE_TOPLEVEL: {
                struct kiwmi_view *view = xdg_surface->data;
                return &view->desktop_surface;
            }
            default:
                return NULL;
            }
        } else if (wlr_surface_is_layer_surface(parent)) {
            struct wlr_layer_surface_v1 *layer_surface =
                wlr_layer_surface_v1_from_wlr_surface(parent);
            struct kiwmi_layer *layer = layer_surface->data;
            return &layer->desktop_surface;
        } else {
            return NULL;
        }
    }
    return NULL;
}

static void
popup_unconstrain(
    struct wlr_xdg_popup *popup,
    struct kiwmi_desktop_surface *desktop_surface)
{
    struct kiwmi_output *output = desktop_surface_get_output(desktop_surface);
    if (!output) {
        return;
    }

    struct wlr_output *wlr_output = output->wlr_output;

    int lx, ly;
    desktop_surface_get_pos(desktop_surface, &lx, &ly);

    if (desktop_surface->type == KIWMI_DESKTOP_SURFACE_VIEW) {
        // wlroots expects surface-local, not view-local coords
        struct kiwmi_view *view =
            wl_container_of(desktop_surface, view, desktop_surface);
        lx -= view->geom.x;
        ly -= view->geom.y;
    }

    double ox = lx;
    double oy = ly;
    wlr_output_layout_output_coords(
        output->desktop->output_layout, wlr_output, &ox, &oy);

    int output_width;
    int output_height;
    wlr_output_effective_resolution(wlr_output, &output_width, &output_height);

    // Relative to the desktop_surface
    struct wlr_box output_box = {
        .x      = -ox,
        .y      = -oy,
        .width  = output_width,
        .height = output_height,
    };

    wlr_xdg_popup_unconstrain_from_box(popup, &output_box);
}

void
popup_attach(
    struct wlr_xdg_popup *popup,
    struct kiwmi_desktop_surface *desktop_surface)
{
    struct wlr_scene_tree *parent_tree = desktop_surface->popups_tree;
    if (wlr_surface_is_xdg_surface(popup->parent)) {
        struct wlr_xdg_surface *xdg_surface =
            wlr_xdg_surface_from_wlr_surface(popup->parent);
        if (xdg_surface->role == WLR_XDG_SURFACE_ROLE_POPUP
            && xdg_surface->data) {
            parent_tree = xdg_surface->data;
        }
    }

    popup_unconstrain(popup, desktop_surface);

    struct wlr_scene_tree *tree =
        wlr_scene_xdg_surface_create(parent_tree, popup->base);
    if (!tree) {
        wlr_log(WLR_ERROR, "failed to attach popup to scene");
        return;
    }
    popup->base->data = tree;
}
