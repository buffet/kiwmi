/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "desktop/output.h"

#include <stdlib.h>

#include <wayland-server.h>
#include <wlr/backend.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_matrix.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_surface.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/util/log.h>

#include "desktop/desktop.h"
#include "desktop/view.h"
#include "input/cursor.h"
#include "input/input.h"
#include "server.h"

struct render_data {
    struct wlr_output *output;
    struct kiwmi_view *view;
    struct wlr_renderer *renderer;
    struct timespec *when;
};

static void
render_surface(struct wlr_surface *surface, int sx, int sy, void *data)
{
    struct render_data *rdata = data;
    struct kiwmi_view *view   = rdata->view;
    struct wlr_output *output = rdata->output;

    struct wlr_texture *texture = wlr_surface_get_texture(surface);
    if (!texture) {
        return;
    }

    double ox = 0;
    double oy = 0;
    wlr_output_layout_output_coords(
        view->desktop->output_layout, output, &ox, &oy);

    ox += view->x + sx;
    oy += view->y + sy;

    struct wlr_box box = {
        .x      = ox * output->scale,
        .y      = oy * output->scale,
        .width  = surface->current.width * output->scale,
        .height = surface->current.height * output->scale,
    };

    float matrix[9];
    enum wl_output_transform transform =
        wlr_output_transform_invert(surface->current.transform);
    wlr_matrix_project_box(
        matrix, &box, transform, 0, output->transform_matrix);

    wlr_render_texture_with_matrix(rdata->renderer, texture, matrix, 1);

    wlr_surface_send_frame_done(surface, rdata->when);
}

static void
output_frame_notify(struct wl_listener *listener, void *data)
{
    struct kiwmi_output *output   = wl_container_of(listener, output, frame);
    struct wlr_output *wlr_output = data;
    struct kiwmi_desktop *desktop = output->desktop;
    struct wlr_renderer *renderer =
        wlr_backend_get_renderer(wlr_output->backend);

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    if (!wlr_output_attach_render(wlr_output, NULL)) {
        wlr_log(WLR_ERROR, "Failed to attach renderer to output");
        return;
    }

    int width;
    int height;

    wlr_output_effective_resolution(wlr_output, &width, &height);

    wlr_renderer_begin(renderer, width, height);
    wlr_renderer_clear(renderer, (float[]){0.0f, 1.0f, 0.0f, 1.0f});

    struct kiwmi_view *view;
    wl_list_for_each_reverse (view, &desktop->views, link) {
        if (view->hidden || !view->mapped) {
            continue;
        }

        struct render_data rdata = {
            .output   = output->wlr_output,
            .view     = view,
            .renderer = renderer,
            .when     = &now,
        };

        view_for_each_surface(view, render_surface, &rdata);
    }

    wlr_output_render_software_cursors(wlr_output, NULL);
    wlr_renderer_end(renderer);

    wlr_output_commit(wlr_output);
}

static void
output_destroy_notify(struct wl_listener *listener, void *UNUSED(data))
{
    struct kiwmi_output *output = wl_container_of(listener, output, destroy);

    wlr_output_layout_remove(
        output->desktop->output_layout, output->wlr_output);

    wl_signal_emit(&output->events.destroy, output);

    wl_list_remove(&output->link);
    wl_list_remove(&output->frame.link);
    wl_list_remove(&output->destroy.link);

    free(output);
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

    output->frame.notify = output_frame_notify;
    wl_signal_add(&wlr_output->events.frame, &output->frame);

    output->destroy.notify = output_destroy_notify;
    wl_signal_add(&wlr_output->events.destroy, &output->destroy);

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
    }

    struct kiwmi_output *output = output_create(wlr_output, desktop);
    if (!output) {
        wlr_log(WLR_ERROR, "Failed to create output");
        return;
    }

    struct kiwmi_cursor *cursor = server->input.cursor;

    wlr_xcursor_manager_load(cursor->xcursor_manager, wlr_output->scale);

    wl_list_insert(&desktop->outputs, &output->link);

    wlr_output_layout_add_auto(desktop->output_layout, wlr_output);

    wlr_output_create_global(wlr_output);

    wl_signal_init(&output->events.destroy);

    wl_signal_emit(&desktop->events.new_output, output);
}
