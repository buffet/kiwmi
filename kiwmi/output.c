/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "kiwmi/output.h"

#include <stdlib.h>

#include <wayland-server.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_output.h>

#include "kiwmi/server.h"

static void output_frame_notify(struct wl_listener *listener, void *data);
static void output_destroy_nofity(struct wl_listener *listener, void *data);

struct kiwmi_output *
output_create(struct wlr_output *wlr_output, struct kiwmi_server *server)
{
    struct kiwmi_output *output = calloc(1, sizeof(*output));
    if (!output) {
        return NULL;
    }

    output->wlr_output = wlr_output;
    output->server     = server;

    output->frame.notify = output_frame_notify;
    wl_signal_add(&wlr_output->events.frame, &output->frame);

    output->destroy.notify = output_destroy_nofity;
    wl_signal_add(&wlr_output->events.destroy, &output->destroy);

    return output;
}

static void
output_frame_notify(struct wl_listener *listener, void *data)
{
    struct kiwmi_output *output   = wl_container_of(listener, output, frame);
    struct wlr_output *wlr_output = data;
    struct wlr_renderer *renderer =
        wlr_backend_get_renderer(wlr_output->backend);

    wlr_output_make_current(wlr_output, NULL);
    wlr_renderer_begin(renderer, wlr_output->width, wlr_output->height);
    {
        float color[] = {0.18f, 0.20f, 0.25f, 1.0f};
        wlr_renderer_clear(renderer, color);
        wlr_output_render_software_cursors(wlr_output, NULL);
        wlr_output_swap_buffers(wlr_output, NULL, NULL);
    }
    wlr_renderer_end(renderer);
}

static void
output_destroy_nofity(struct wl_listener *listener, void *UNUSED(data))
{
    struct kiwmi_output *output = wl_container_of(listener, output, destroy);

    wlr_output_layout_remove(output->server->output_layout, output->wlr_output);

    wl_list_remove(&output->link);
    wl_list_remove(&output->frame.link);
    wl_list_remove(&output->destroy.link);

    free(output);
}
