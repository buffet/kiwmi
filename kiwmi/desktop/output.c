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
#include <wlr/types/wlr_layer_shell_v1.h>
#include <wlr/types/wlr_matrix.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_output_management_v1.h>
#include <wlr/types/wlr_surface.h>
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
            view_for_each_surface(view, send_frame_done_to_surface, &now);
        }

        if (render_cursors(wlr_output)) {
            output_damage(output);
        }

        wlr_output_commit(wlr_output);
        return;
    }

    struct wlr_output_layout *output_layout = desktop->output_layout;
    struct kiwmi_server *server   = wl_container_of(desktop, server, desktop);
    struct wlr_renderer *renderer = server->renderer;

    wlr_renderer_begin(renderer, wlr_output->width, wlr_output->height);
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
        view_for_each_surface(view, render_surface, &rdata);
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

void
output_manager_update(struct kiwmi_desktop *desktop)
{
    struct wlr_output_configuration_v1 *configuration =
        wlr_output_configuration_v1_create();

    struct kiwmi_output *output;
    wl_list_for_each (output, &desktop->outputs, link) {
        struct wlr_output_configuration_head_v1 *head =
            wlr_output_configuration_head_v1_create(
                configuration, output->wlr_output);

        struct wlr_box *area = wlr_output_layout_get_box(
            desktop->output_layout, output->wlr_output);

        if (area != NULL) {
            head->state.x = area->x;
            head->state.y = area->y;
        }
    }

    wlr_output_manager_v1_set_configuration(
        desktop->output_manager, configuration);
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
        output_manager_update(output->desktop);
    }

    wl_signal_emit(&output->events.destroy, output);

    wl_list_remove(&output->link);
    wl_list_remove(&output->frame.link);
    wl_list_remove(&output->commit.link);
    wl_list_remove(&output->destroy.link);
    wl_list_remove(&output->mode.link);

    wl_list_remove(&output->events.destroy.listener_list);

    int n_layers = sizeof(output->layers) / sizeof(output->layers[0]);
    for (int i = 0; i < n_layers; i++) {
        struct kiwmi_layer *layer;
        struct kiwmi_layer *tmp;
        wl_list_for_each_safe (layer, tmp, &output->layers[i], link) {
            wl_list_remove(&layer->link);
            layer->output = NULL;
            wlr_layer_surface_v1_destroy(layer->layer_surface);
        }
    }

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
        if (pointer->device->output_name
            && strcmp(pointer->device->output_name, wlr_output->name) == 0) {
            wlr_cursor_map_input_to_output(
                cursor->cursor, pointer->device, wlr_output);
        }
    }

    wl_list_insert(&desktop->outputs, &output->link);

    wlr_output_layout_add_auto(desktop->output_layout, wlr_output);
    output_manager_update(desktop);

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

static void
output_manager_configure(
    struct kiwmi_server *server,
    struct wlr_output_configuration_v1 *configuration,
    bool test)
{
    struct wlr_output_configuration_head_v1 *head;
    struct wlr_output_configuration_head_v1 *bad_head = NULL;
    wl_list_for_each (head, &configuration->heads, link) {
        struct wlr_output *wlr_output = head->state.output;

        wlr_output_enable(wlr_output, head->state.enabled);
        if (head->state.enabled) {
            if (head->state.mode) {
                wlr_output_set_mode(wlr_output, head->state.mode);
            } else {
                wlr_output_set_custom_mode(
                    wlr_output,
                    head->state.custom_mode.width,
                    head->state.custom_mode.height,
                    head->state.custom_mode.refresh);
            }

            wlr_output_set_transform(wlr_output, head->state.transform);
            wlr_output_set_scale(wlr_output, head->state.scale);
        }

        if (!wlr_output_test(wlr_output)) {
            bad_head = head;
            break;
        }
    }

    wl_list_for_each (head, &configuration->heads, link) {
        if (bad_head != NULL || test) {
            wlr_output_rollback(head->state.output);
            if (head == bad_head) {
                break;
            } else {
                continue;
            }
        }
        struct kiwmi_output *output = head->state.output->data;
        bool enable_changed         = WLR_OUTPUT_STATE_ENABLED
            == (output->wlr_output->pending.committed
                & WLR_OUTPUT_STATE_ENABLED);
        if (head->state.enabled) {
            if (enable_changed) {
                wlr_output_layout_add(
                    server->desktop.output_layout,
                    output->wlr_output,
                    head->state.x,
                    head->state.y);
                wl_signal_emit(&output->desktop->events.new_output, output);
            } else {
                bool moved = output->wlr_output->pending.committed
                    & (WLR_OUTPUT_STATE_MODE | WLR_OUTPUT_STATE_SCALE
                       | WLR_OUTPUT_STATE_TRANSFORM);
                struct wlr_output_layout_output *layout_output =
                    wlr_output_layout_get(
                        server->desktop.output_layout, output->wlr_output);
                moved |= head->state.x != layout_output->x
                    || head->state.y != layout_output->y;
                if (moved) {
                    wlr_output_layout_move(
                        server->desktop.output_layout,
                        output->wlr_output,
                        head->state.x,
                        head->state.y);
                    wl_signal_emit(&output->events.resize, output);
                }
            }
        } else if (enable_changed) {
            wl_signal_emit(&output->events.destroy, output);
            wlr_output_layout_remove(
                server->desktop.output_layout, output->wlr_output);
        }
        wlr_output_commit(head->state.output);
    }

    if (bad_head == NULL) {
        wlr_output_configuration_v1_send_succeeded(configuration);
        if (!test) {
            wlr_output_manager_v1_set_configuration(
                server->desktop.output_manager, configuration);
        } else {
            wlr_output_configuration_v1_destroy(configuration);
        }
    } else {
        wlr_output_configuration_v1_send_failed(configuration);
        wlr_output_configuration_v1_destroy(configuration);
    }
}

void
output_manager_apply_notify(struct wl_listener *listener, void *data)
{
    struct kiwmi_desktop *desktop =
        wl_container_of(listener, desktop, output_manager_apply);
    struct kiwmi_server *server = wl_container_of(desktop, server, desktop);
    struct wlr_output_configuration_v1 *configuration = data;

    output_manager_configure(server, configuration, false);
}

void
output_manager_test_notify(struct wl_listener *listener, void *data)
{
    struct kiwmi_desktop *desktop =
        wl_container_of(listener, desktop, output_manager_test);
    struct kiwmi_server *server = wl_container_of(desktop, server, desktop);
    struct wlr_output_configuration_v1 *configuration = data;

    output_manager_configure(server, configuration, true);
}

void
output_damage(struct kiwmi_output *output)
{
    if (output != NULL) {
        output->damaged = 2;
    }
}
