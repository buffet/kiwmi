/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef KIWMI_DESKTOP_OUTPUT_H
#define KIWMI_DESKTOP_OUTPUT_H

#include <wayland-server.h>
#include <wlr/util/box.h>

struct kiwmi_output {
    struct wl_list link;
    struct kiwmi_desktop *desktop;
    struct wlr_output *wlr_output;
    struct wl_listener frame;
    struct wl_listener commit;
    struct wl_listener destroy;
    struct wl_listener mode;

    struct wl_list layers[4]; // struct kiwmi_layer_surface::link
    struct wlr_box usable_area;

    int damaged;

    struct {
        struct wl_signal destroy;
        struct wl_signal resize;
        struct wl_signal usable_area_change;
    } events;
};

struct kiwmi_render_data {
    struct wlr_output *output;
    double output_lx;
    double output_ly;
    struct wlr_renderer *renderer;
    struct timespec *when;
    void *data;
};

void new_output_notify(struct wl_listener *listener, void *data);
void output_layout_change_notify(struct wl_listener *listener, void *data);
void output_manager_apply_notify(struct wl_listener *listener, void *data);
void output_manager_test_notify(struct wl_listener *listener, void *data);

void output_damage(struct kiwmi_output *output);

#endif /* KIWMI_DESKTOP_OUTPUT_H */
