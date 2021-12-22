/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "desktop/popup.h"

#include <wlr/types/wlr_layer_shell_v1.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/util/log.h>

#include "desktop/desktop_surface.h"
#include "desktop/layer_shell.h"
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
            case WLR_XDG_SURFACE_ROLE_TOPLEVEL:
                struct kiwmi_view *view = xdg_surface->data;
                return &view->desktop_surface;
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

    struct wlr_scene_node *node =
        wlr_scene_xdg_surface_create(&parent_tree->node, popup->base);
    if (!node) {
        wlr_log(WLR_ERROR, "failed to attach popup to scene");
        return;
    }
    popup->base->data = node;
}
