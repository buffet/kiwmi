/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "desktop/output.h"

#include <stdlib.h>

#include <pixman.h>
#include <wayland-server.h>
#include <wlr/backend.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_layer_shell_v1.h>
#include <wlr/types/wlr_matrix.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/util/log.h>

#include "desktop/desktop.h"
#include "desktop/layer_shell.h"
#include "desktop/view.h"
#include "input/cursor.h"
#include "input/input.h"
#include "input/pointer.h"
#include "server.h"

static void
output_frame_notify(struct wl_listener *listener, void *data)
{
    struct kiwmi_output *output   = wl_container_of(listener, output, frame);
    struct wlr_output *wlr_output = data;

    struct wlr_scene_output *scene_output =
        wlr_scene_get_scene_output(output->desktop->scene, wlr_output);

    if (!scene_output) {
        return;
    }

    wlr_scene_output_commit(scene_output);

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    wlr_scene_output_send_frame_done(scene_output, &now);
}

static void
output_commit_notify(struct wl_listener *listener, void *data)
{
    struct kiwmi_output *output = wl_container_of(listener, output, commit);
    struct wlr_output_event_commit *event = data;

    if (event->committed & WLR_OUTPUT_STATE_TRANSFORM) {
        arrange_layers(output);

        wl_signal_emit(&output->events.resize, output);
    }
}

static void
output_destroy_notify(struct wl_listener *listener, void *UNUSED(data))
{
    struct kiwmi_output *output = wl_container_of(listener, output, destroy);

    wl_signal_emit(&output->events.destroy, output);

    int n_layers = sizeof(output->layers) / sizeof(output->layers[0]);
    for (int i = 0; i < n_layers; i++) {
        struct kiwmi_layer *layer;
        struct kiwmi_layer *tmp;
        wl_list_for_each_safe (layer, tmp, &output->layers[i], link) {
            // Set output to NULL to avoid rearranging the remaining layers.
            // Our impl requires that we remove the layer from the list.
            layer->output = NULL;
            wl_list_remove(&layer->link);
            wlr_layer_surface_v1_destroy(layer->layer_surface);
        }
    }

    if (output->desktop->scene) {
        for (size_t i = 0; i < KIWMI_STRATA_COUNT; ++i) {
            wlr_scene_node_destroy(&output->strata[i]->node);
        }
    }

    if (output->desktop->output_layout) {
        wlr_output_layout_remove(
            output->desktop->output_layout, output->wlr_output);
    }

    wl_list_remove(&output->link);
    wl_list_remove(&output->frame.link);
    wl_list_remove(&output->commit.link);
    wl_list_remove(&output->destroy.link);
    wl_list_remove(&output->mode.link);

    wl_list_remove(&output->events.destroy.listener_list);

    free(output);
}

static void
output_mode_notify(struct wl_listener *listener, void *UNUSED(data))
{
    struct kiwmi_output *output = wl_container_of(listener, output, mode);

    arrange_layers(output);

    wl_signal_emit(&output->events.resize, output);
}

static struct kiwmi_output *
output_create(struct wlr_output *wlr_output, struct kiwmi_desktop *desktop)
{
    struct kiwmi_output *output = calloc(1, sizeof(*output));
    if (!output) {
        return NULL;
    }

    output->wlr_output = wlr_output;
    output->desktop    = desktop;

    output->usable_area.width  = wlr_output->width;
    output->usable_area.height = wlr_output->height;

    output->frame.notify = output_frame_notify;
    wl_signal_add(&wlr_output->events.frame, &output->frame);

    output->commit.notify = output_commit_notify;
    wl_signal_add(&wlr_output->events.commit, &output->commit);

    output->destroy.notify = output_destroy_notify;
    wl_signal_add(&wlr_output->events.destroy, &output->destroy);

    output->mode.notify = output_mode_notify;
    wl_signal_add(&wlr_output->events.mode, &output->mode);

    return output;
}

void
new_output_notify(struct wl_listener *listener, void *data)
{
    struct kiwmi_desktop *desktop =
        wl_container_of(listener, desktop, new_output);
    struct kiwmi_server *server   = wl_container_of(desktop, server, desktop);
    struct wlr_output *wlr_output = data;

    wlr_log(WLR_DEBUG, "New output %p: %s", wlr_output, wlr_output->name);

    wlr_output_init_render(wlr_output, server->allocator, server->renderer);

    struct wlr_output_mode *mode = wlr_output_preferred_mode(wlr_output);
    if (mode) {
        wlr_output_set_mode(wlr_output, mode);

        if (!wlr_output_commit(wlr_output)) {
            wlr_log(WLR_ERROR, "Failed to modeset output");
            return;
        }
    }

    struct kiwmi_output *output = output_create(wlr_output, desktop);
    if (!output) {
        wlr_log(WLR_ERROR, "Failed to create output");
        return;
    }

    wlr_output->data = output;

    struct kiwmi_cursor *cursor = server->input.cursor;
    wlr_xcursor_manager_load(cursor->xcursor_manager, wlr_output->scale);

    struct kiwmi_pointer *pointer;
    wl_list_for_each (pointer, &server->input.pointers, link) {
        if (pointer->pointer->base.name
            && strcmp(pointer->pointer->base.name, wlr_output->name) == 0) {
            wlr_cursor_map_input_to_output(
                cursor->cursor, &pointer->pointer->base, wlr_output);
        }
    }

    wl_list_insert(&desktop->outputs, &output->link);

    wlr_output_layout_add_auto(desktop->output_layout, wlr_output);

    wlr_output_create_global(wlr_output);

    size_t len_layers = sizeof(output->layers) / sizeof(output->layers[0]);
    for (size_t i = 0; i < len_layers; ++i) {
        wl_list_init(&output->layers[i]);
    }

    for (size_t i = 0; i < KIWMI_STRATA_COUNT; ++i) {
        output->strata[i] = wlr_scene_tree_create(desktop->strata[i]);
    }

    wl_signal_init(&output->events.destroy);
    wl_signal_init(&output->events.resize);
    wl_signal_init(&output->events.usable_area_change);

    wl_list_insert(&desktop->outputs, &output->link);

    wlr_output_layout_add_auto(desktop->output_layout, wlr_output);

    wl_signal_emit(&desktop->events.new_output, output);
}

void
output_layout_change_notify(struct wl_listener *listener, void *UNUSED(data))
{
    struct kiwmi_desktop *desktop =
        wl_container_of(listener, desktop, output_layout_change);

    struct wlr_box ol_box;
    wlr_output_layout_get_box(desktop->output_layout, NULL, &ol_box);
    wlr_scene_node_set_position(
        &desktop->background_rect->node, ol_box.x, ol_box.y);
    wlr_scene_rect_set_size(
        desktop->background_rect, ol_box.width, ol_box.height);

    struct wlr_output_layout_output *ol_output;
    wl_list_for_each (ol_output, &desktop->output_layout->outputs, link) {
        struct kiwmi_output *output = ol_output->output->data;

        struct wlr_box box;
        wlr_output_layout_get_box(
            desktop->output_layout, output->wlr_output, &box);

        for (size_t i = 0; i < KIWMI_STRATA_COUNT; ++i) {
            if (output->strata[i]) {
                wlr_scene_node_set_position(
                    &output->strata[i]->node, box.x, box.y);
            }
        }
    }
}
