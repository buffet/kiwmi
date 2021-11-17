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
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_layer_shell_v1.h>
#include <wlr/types/wlr_matrix.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_surface.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/util/log.h>

#include "desktop/desktop.h"
#include "desktop/layer_shell.h"
#include "desktop/view.h"
#include "input/cursor.h"
#include "input/input.h"
#include "server.h"

static void
render_layer_surface(struct wlr_surface *surface, int x, int y, void *data)
{
    struct kiwmi_render_data *rdata = data;
    struct wlr_output *wlr_output   = rdata->output;
    struct wlr_box *geom            = rdata->data;

    struct wlr_texture *texture = wlr_surface_get_texture(surface);
    if (!texture) {
        return;
    }

    int ox = x + geom->x;
    int oy = y + geom->y;

    struct wlr_box box = {
        .x      = ox * wlr_output->scale,
        .y      = oy * wlr_output->scale,
        .width  = surface->current.width * wlr_output->scale,
        .height = surface->current.height * wlr_output->scale,
    };

    float matrix[9];
    enum wl_output_transform transform =
        wlr_output_transform_invert(surface->current.transform);
    wlr_matrix_project_box(
        matrix, &box, transform, 0, wlr_output->transform_matrix);

    wlr_render_texture_with_matrix(rdata->renderer, texture, matrix, 1);

    wlr_surface_send_frame_done(surface, rdata->when);
}

static void
render_layer(struct wl_list *layer, struct kiwmi_render_data *rdata)
{
    struct kiwmi_layer *surface;
    wl_list_for_each (surface, layer, link) {
        rdata->data = &surface->geom;

        wlr_layer_surface_v1_for_each_surface(
            surface->layer_surface, render_layer_surface, rdata);
    }
}

static void
send_frame_done_to_layer_surface(
    struct wlr_surface *surface,
    int UNUSED(x),
    int UNUSED(y),
    void *data)
{
    struct timespec *now = data;
    wlr_surface_send_frame_done(surface, now);
}

static void
send_frame_done_to_layer(struct wl_list *layer, struct timespec *now)
{
    struct kiwmi_layer *surface;
    wl_list_for_each (surface, layer, link) {
        wlr_layer_surface_v1_for_each_surface(
            surface->layer_surface, send_frame_done_to_layer_surface, now);
    }
}

static void
render_surface(struct wlr_surface *surface, int sx, int sy, void *data)
{
    struct kiwmi_render_data *rdata = data;
    struct kiwmi_view *view         = rdata->data;
    struct wlr_output *wlr_output   = rdata->output;

    struct wlr_texture *texture = wlr_surface_get_texture(surface);
    if (!texture) {
        return;
    }

    int ox = rdata->output_lx + sx + view->x - view->geom.x;
    int oy = rdata->output_ly + sy + view->y - view->geom.y;

    struct wlr_box box = {
        .x      = ox * wlr_output->scale,
        .y      = oy * wlr_output->scale,
        .width  = surface->current.width * wlr_output->scale,
        .height = surface->current.height * wlr_output->scale,
    };

    float matrix[9];
    enum wl_output_transform transform =
        wlr_output_transform_invert(surface->current.transform);
    wlr_matrix_project_box(
        matrix, &box, transform, 0, wlr_output->transform_matrix);

    wlr_render_texture_with_matrix(rdata->renderer, texture, matrix, 1);

    wlr_surface_send_frame_done(surface, rdata->when);
}

static void
send_frame_done_to_surface(
    struct wlr_surface *surface,
    int UNUSED(sx),
    int UNUSED(sy),
    void *data)
{
    struct timespec *now = data;
    wlr_surface_send_frame_done(surface, now);
}

static bool
render_cursors(struct wlr_output *wlr_output)
{
    pixman_region32_t damage;
    pixman_region32_init(&damage);
    wlr_output_render_software_cursors(wlr_output, &damage);
    bool damaged = pixman_region32_not_empty(&damage);
    pixman_region32_fini(&damage);

    return damaged;
}

static void
output_frame_notify(struct wl_listener *listener, void *data)
{
    struct kiwmi_output *output   = wl_container_of(listener, output, frame);
    struct wlr_output *wlr_output = data;
    struct kiwmi_desktop *desktop = output->desktop;

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    int buffer_age;
    if (!wlr_output_attach_render(wlr_output, &buffer_age)) {
        wlr_log(WLR_ERROR, "Failed to attach renderer to output");
        return;
    }

    if (output->damaged == 0 && buffer_age > 0) {
        send_frame_done_to_layer(
            &output->layers[ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND], &now);
        send_frame_done_to_layer(
            &output->layers[ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM], &now);
        send_frame_done_to_layer(
            &output->layers[ZWLR_LAYER_SHELL_V1_LAYER_TOP], &now);
        send_frame_done_to_layer(
            &output->layers[ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY], &now);

        struct kiwmi_view *view;
        wl_list_for_each (view, &desktop->views, link) {
            view_for_each_mapped_surface(
                view, send_frame_done_to_surface, &now);
        }

        if (render_cursors(wlr_output)) {
            output_damage(output);
        }

        wlr_output_commit(wlr_output);
        return;
    }

    struct wlr_output_layout *output_layout = desktop->output_layout;
    struct wlr_renderer *renderer =
        wlr_backend_get_renderer(wlr_output->backend);

    int width;
    int height;
    wlr_output_effective_resolution(wlr_output, &width, &height);

    wlr_renderer_begin(renderer, width, height);
    wlr_renderer_clear(renderer, desktop->bg_color);

    double output_lx = 0;
    double output_ly = 0;
    wlr_output_layout_output_coords(
        output_layout, wlr_output, &output_lx, &output_ly);

    struct kiwmi_render_data rdata = {
        .output    = output->wlr_output,
        .output_lx = output_lx,
        .output_ly = output_ly,
        .renderer  = renderer,
        .when      = &now,
    };

    render_layer(&output->layers[ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND], &rdata);
    render_layer(&output->layers[ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM], &rdata);

    struct kiwmi_view *view;
    wl_list_for_each_reverse (view, &desktop->views, link) {
        if (view->hidden || !view->mapped) {
            continue;
        }

        rdata.data = view;

        wl_signal_emit(&view->events.pre_render, &rdata);
        view_for_each_mapped_surface(view, render_surface, &rdata);
        wl_signal_emit(&view->events.post_render, &rdata);
    }

    render_layer(&output->layers[ZWLR_LAYER_SHELL_V1_LAYER_TOP], &rdata);
    render_layer(&output->layers[ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY], &rdata);

    bool damaged = render_cursors(wlr_output);
    wlr_renderer_end(renderer);

    if (damaged) {
        output_damage(output);
    } else {
        --output->damaged;
    }

    wlr_output_commit(wlr_output);
}

static void
output_commit_notify(struct wl_listener *listener, void *data)
{
    struct kiwmi_output *output = wl_container_of(listener, output, commit);
    struct wlr_output_event_commit *event = data;

    if (event->committed & WLR_OUTPUT_STATE_TRANSFORM) {
        arrange_layers(output);
        output_damage(output);

        wl_signal_emit(&output->events.resize, output);
    }
}

static void
output_destroy_notify(struct wl_listener *listener, void *UNUSED(data))
{
    struct kiwmi_output *output = wl_container_of(listener, output, destroy);

    if (output->desktop->output_layout) {
        wlr_output_layout_remove(
            output->desktop->output_layout, output->wlr_output);
    }

    wl_signal_emit(&output->events.destroy, output);

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
    output_damage(output);

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

    output_damage(output);

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

    if (!wl_list_empty(&wlr_output->modes)) {
        struct wlr_output_mode *mode =
            wl_container_of(wlr_output->modes.prev, mode, link);
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

    wl_list_insert(&desktop->outputs, &output->link);

    wlr_output_layout_add_auto(desktop->output_layout, wlr_output);

    wlr_output_create_global(wlr_output);

    size_t len_outputs = sizeof(output->layers) / sizeof(output->layers[0]);
    for (size_t i = 0; i < len_outputs; ++i) {
        wl_list_init(&output->layers[i]);
    }

    wl_signal_init(&output->events.destroy);
    wl_signal_init(&output->events.resize);
    wl_signal_init(&output->events.usable_area_change);

    wl_signal_emit(&desktop->events.new_output, output);
}

void
output_damage(struct kiwmi_output *output)
{
    output->damaged = 2;
}
