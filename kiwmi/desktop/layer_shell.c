/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "desktop/layer_shell.h"

#include <stdlib.h>

#include <pixman.h>
#include <wayland-server.h>
#include <wlr/types/wlr_layer_shell_v1.h>
#include <wlr/util/log.h>

#include "desktop/desktop.h"
#include "desktop/desktop_surface.h"
#include "desktop/output.h"
#include "desktop/stratum.h"
#include "input/seat.h"
#include "server.h"

static void
kiwmi_layer_destroy_notify(struct wl_listener *listener, void *UNUSED(data))
{
    struct kiwmi_layer *layer = wl_container_of(listener, layer, destroy);

    wlr_scene_node_destroy(&layer->desktop_surface.tree->node);
    wlr_scene_node_destroy(&layer->desktop_surface.popups_tree->node);

    wl_list_remove(&layer->destroy.link);
    wl_list_remove(&layer->map.link);
    wl_list_remove(&layer->unmap.link);

    if (layer->output != NULL) {
        wl_list_remove(&layer->link);
        arrange_layers(layer->output);
    }

    free(layer);
}

static void
kiwmi_layer_commit_notify(struct wl_listener *listener, void *UNUSED(data))
{
    struct kiwmi_layer *layer   = wl_container_of(listener, layer, commit);
    struct kiwmi_output *output = layer->output;

    if (layer->layer != layer->layer_surface->current.layer) {
        wl_list_remove(&layer->link);
        layer->layer = layer->layer_surface->current.layer;
        wl_list_insert(&output->layers[layer->layer], &layer->link);

        enum kiwmi_stratum new_stratum =
            stratum_from_layer_shell_layer(layer->layer);

        wlr_scene_node_reparent(
            &layer->desktop_surface.tree->node, output->strata[new_stratum]);
    }

    if (layer->layer_surface->current.committed != 0) {
        arrange_layers(output);
    }
}

static void
kiwmi_layer_map_notify(struct wl_listener *listener, void *UNUSED(data))
{
    struct kiwmi_layer *layer = wl_container_of(listener, layer, map);

    wlr_scene_node_set_enabled(&layer->desktop_surface.tree->node, true);
    wlr_scene_node_set_enabled(&layer->desktop_surface.popups_tree->node, true);

    arrange_layers(layer->output);
}

static void
kiwmi_layer_unmap_notify(struct wl_listener *listener, void *UNUSED(data))
{
    struct kiwmi_layer *layer = wl_container_of(listener, layer, unmap);

    wlr_scene_node_set_enabled(&layer->desktop_surface.tree->node, false);
    wlr_scene_node_set_enabled(
        &layer->desktop_surface.popups_tree->node, false);

    arrange_layers(layer->output);
}

static void
apply_exclusive(
    struct wlr_box *usable_area,
    uint32_t anchor,
    int32_t exclusive,
    int32_t margin_top,
    int32_t margin_bottom,
    int32_t margin_left,
    int32_t margin_right)
{
    if (exclusive <= 0) {
        return;
    }

    struct {
        uint32_t anchors;
        int *positive_axis;
        int *negative_axis;
        int margin;
    } edges[] = {
        {
            .anchors = ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT
                | ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT
                | ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP,
            .positive_axis = &usable_area->y,
            .negative_axis = &usable_area->height,
            .margin        = margin_top,
        },
        {
            .anchors = ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT
                | ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT
                | ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM,
            .positive_axis = NULL,
            .negative_axis = &usable_area->height,
            .margin        = margin_bottom,
        },
        {
            .anchors = ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT
                | ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP
                | ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM,
            .positive_axis = &usable_area->x,
            .negative_axis = &usable_area->width,
            .margin        = margin_left,
        },
        {
            .anchors = ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT
                | ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP
                | ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM,
            .positive_axis = NULL,
            .negative_axis = &usable_area->width,
            .margin        = margin_right,
        },
    };

    size_t nedges = sizeof(edges) / sizeof(edges[0]);
    for (size_t i = 0; i < nedges; ++i) {
        if ((anchor & edges[i].anchors) == edges[i].anchors
            && exclusive + edges[i].margin > 0) {
            if (edges[i].positive_axis) {
                *edges[i].positive_axis += exclusive + edges[i].margin;
            }
            if (edges[i].negative_axis) {
                *edges[i].negative_axis -= exclusive + edges[i].margin;
            }
        }
    }
}

static void
arrange_layer(
    struct kiwmi_output *output,
    struct wl_list *layers,
    struct wlr_box *usable_area,
    bool exclusive)
{
    struct wlr_box full_area = {0};

    wlr_output_effective_resolution(
        output->wlr_output, &full_area.width, &full_area.height);

    struct kiwmi_layer *layer;
    wl_list_for_each_reverse (layer, layers, link) {
        struct wlr_layer_surface_v1 *layer_surface = layer->layer_surface;
        struct wlr_layer_surface_v1_state *state   = &layer_surface->current;

        if (exclusive != (state->exclusive_zone >= 0)) {
            continue;
        }

        struct wlr_box bounds;

        if (state->exclusive_zone == -1) {
            bounds = full_area;
        } else {
            bounds = *usable_area;
        }

        struct wlr_box arranged_area = {
            .width  = state->desired_width,
            .height = state->desired_height,
        };

        // horizontal
        const uint32_t both_horiz = ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT
            | ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT;

        if ((state->anchor & both_horiz) && arranged_area.width == 0) {
            arranged_area.x     = bounds.x;
            arranged_area.width = bounds.width;
        } else if (state->anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT) {
            arranged_area.x = bounds.x;
        } else if (state->anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT) {
            arranged_area.x = bounds.x + (bounds.width - arranged_area.width);
        } else {
            arranged_area.x =
                bounds.x + ((bounds.width / 2) - (arranged_area.width / 2));
        }

        // vertical
        const uint32_t both_vert = ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP
            | ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM;
        if ((state->anchor & both_vert) && arranged_area.height == 0) {
            arranged_area.y      = bounds.y;
            arranged_area.height = bounds.height;
        } else if (state->anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP) {
            arranged_area.y = bounds.y;
        } else if (state->anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM) {
            arranged_area.y = bounds.y + (bounds.height - arranged_area.height);
        } else {
            arranged_area.y =
                bounds.y + ((bounds.height / 2) - (arranged_area.height / 2));
        }

        // left and right margin
        if ((state->anchor & both_horiz) == both_horiz) {
            arranged_area.x += state->margin.left;
            arranged_area.width -= state->margin.left + state->margin.right;
        } else if (state->anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT) {
            arranged_area.x += state->margin.left;
        } else if (state->anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT) {
            arranged_area.x -= state->margin.right;
        }

        // top and bottom margin
        if ((state->anchor & both_vert) == both_vert) {
            arranged_area.y += state->margin.top;
            arranged_area.height -= state->margin.top + state->margin.bottom;
        } else if (state->anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP) {
            arranged_area.y += state->margin.top;
        } else if (state->anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM) {
            arranged_area.y -= state->margin.bottom;
        }

        if (arranged_area.width < 0 || arranged_area.height < 0) {
            wlr_log(
                WLR_ERROR,
                "Bad width/height: %d, %d",
                arranged_area.width,
                arranged_area.height);
            wlr_layer_surface_v1_destroy(layer_surface);
            continue;
        }

        wlr_scene_node_set_position(
            &layer->desktop_surface.tree->node,
            arranged_area.x,
            arranged_area.y);
        wlr_scene_node_set_position(
            &layer->desktop_surface.popups_tree->node,
            arranged_area.x,
            arranged_area.y);

        apply_exclusive(
            usable_area,
            state->anchor,
            state->exclusive_zone,
            state->margin.top,
            state->margin.bottom,
            state->margin.left,
            state->margin.right);

        wlr_layer_surface_v1_configure(
            layer_surface, arranged_area.width, arranged_area.height);
    }
}

void
arrange_layers(struct kiwmi_output *output)
{
    struct wlr_box usable_area = {0};

    wlr_output_effective_resolution(
        output->wlr_output, &usable_area.width, &usable_area.height);

    // arrange exclusive layers
    arrange_layer(
        output,
        &output->layers[ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY],
        &usable_area,
        true);
    arrange_layer(
        output,
        &output->layers[ZWLR_LAYER_SHELL_V1_LAYER_TOP],
        &usable_area,
        true);
    arrange_layer(
        output,
        &output->layers[ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM],
        &usable_area,
        true);
    arrange_layer(
        output,
        &output->layers[ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND],
        &usable_area,
        true);

    // arrange non-exclusive layers
    arrange_layer(
        output,
        &output->layers[ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY],
        &usable_area,
        false);
    arrange_layer(
        output,
        &output->layers[ZWLR_LAYER_SHELL_V1_LAYER_TOP],
        &usable_area,
        false);
    arrange_layer(
        output,
        &output->layers[ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM],
        &usable_area,
        false);
    arrange_layer(
        output,
        &output->layers[ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND],
        &usable_area,
        false);

    uint32_t layers_above_shell[] = {
        ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY,
        ZWLR_LAYER_SHELL_V1_LAYER_TOP,
    };
    size_t nlayers = sizeof(layers_above_shell) / sizeof(layers_above_shell[0]);
    struct kiwmi_layer *layer;
    struct kiwmi_layer *topmost = NULL;
    for (size_t i = 0; i < nlayers; ++i) {
        wl_list_for_each_reverse (
            layer, &output->layers[layers_above_shell[i]], link) {
            if (layer->layer_surface->current.keyboard_interactive) {
                topmost = layer;
                break;
            }
        }

        if (topmost) {
            break;
        }
    }

    if (memcmp(&usable_area, &output->usable_area, sizeof(output->usable_area))
        != 0) {
        memcpy(&output->usable_area, &usable_area, sizeof(output->usable_area));
        wl_signal_emit(&output->events.usable_area_change, output);
    }

    struct kiwmi_desktop *desktop = output->desktop;
    struct kiwmi_server *server   = wl_container_of(desktop, server, desktop);
    struct kiwmi_seat *seat       = server->input.seat;

    seat_focus_layer(seat, topmost);
}

static struct kiwmi_output *
layer_desktop_surface_get_output(struct kiwmi_desktop_surface *desktop_surface)
{
    struct kiwmi_layer *layer =
        wl_container_of(desktop_surface, layer, desktop_surface);
    return layer->output;
}

static const struct kiwmi_desktop_surface_impl layer_desktop_surface_impl = {
    .get_output = layer_desktop_surface_get_output,
};

void
layer_shell_new_surface_notify(struct wl_listener *listener, void *data)
{
    struct kiwmi_desktop *desktop =
        wl_container_of(listener, desktop, layer_shell_new_surface);
    struct wlr_layer_surface_v1 *layer_surface = data;

    wlr_log(
        WLR_DEBUG,
        "New layer_shell surface namespace='%s'",
        layer_surface->namespace);

    if (!layer_surface->output) {
        struct kiwmi_server *server = wl_container_of(desktop, server, desktop);
        layer_surface->output       = desktop_active_output(server)->wlr_output;
    }

    struct kiwmi_layer *layer = malloc(sizeof(*layer));
    if (!layer) {
        wlr_log(WLR_ERROR, "Failed too allocate kiwmi_layer_shell");
        return;
    }

    struct kiwmi_output *output = layer_surface->output->data;

    size_t len = sizeof(output->layers) / sizeof(output->layers[0]);
    if (layer_surface->current.layer >= len) {
        wlr_log(
            WLR_ERROR,
            "Bad layer surface layer '%d'",
            layer_surface->current.layer);
        wlr_layer_surface_v1_destroy(layer_surface);
        free(layer);
        return;
    }

    layer->layer_surface = layer_surface;
    layer->output        = output;
    layer->layer         = layer_surface->current.layer;

    layer->desktop_surface.type = KIWMI_DESKTOP_SURFACE_LAYER;
    layer->desktop_surface.impl = &layer_desktop_surface_impl;

    layer->destroy.notify = kiwmi_layer_destroy_notify;
    wl_signal_add(&layer_surface->events.destroy, &layer->destroy);

    layer->commit.notify = kiwmi_layer_commit_notify;
    wl_signal_add(&layer_surface->surface->events.commit, &layer->commit);

    layer->map.notify = kiwmi_layer_map_notify;
    wl_signal_add(&layer_surface->events.map, &layer->map);

    layer->unmap.notify = kiwmi_layer_unmap_notify;
    wl_signal_add(&layer_surface->events.unmap, &layer->unmap);

    layer_surface->data = layer;

    enum kiwmi_stratum stratum = stratum_from_layer_shell_layer(layer->layer);

    layer->desktop_surface.tree =
        wlr_scene_tree_create(output->strata[stratum]);
    layer->desktop_surface.popups_tree =
        wlr_scene_tree_create(output->strata[KIWMI_STRATUM_POPUPS]);
    layer->desktop_surface.surface_tree = wlr_scene_subsurface_tree_create(
        layer->desktop_surface.tree, layer->layer_surface->surface);

    wlr_scene_node_set_enabled(&layer->desktop_surface.tree->node, false);
    wlr_scene_node_set_enabled(
        &layer->desktop_surface.popups_tree->node, false);

    wl_list_insert(&output->layers[layer->layer], &layer->link);

    // Temporarily set the layer's current state to pending
    // so that we can easily arrange it
    struct wlr_layer_surface_v1_state old_state = layer_surface->current;
    layer_surface->current                      = layer_surface->pending;
    arrange_layers(output);
    layer_surface->current = old_state;
}
