/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef KIWMI_DESKTOP_DESKTOP_SURFACE_H
#define KIWMI_DESKTOP_DESKTOP_SURFACE_H

struct wlr_surface;
struct kiwmi_desktop;

enum kiwmi_desktop_surface_type {
    KIWMI_DESKTOP_SURFACE_VIEW,
    KIWMI_DESKTOP_SURFACE_LAYER,
};

struct kiwmi_desktop_surface {
    // The tree is where the config is supposed to put custom decorations (it
    // also contains the surface_node)
    struct wlr_scene_tree *tree;
    struct wlr_scene_tree *surface_tree;

    struct wlr_scene_tree *popups_tree;

    enum kiwmi_desktop_surface_type type;
    const struct kiwmi_desktop_surface_impl *impl;
};

struct kiwmi_desktop_surface_impl {
    struct kiwmi_output *(*get_output)(
        struct kiwmi_desktop_surface *desktop_surface);
};

struct kiwmi_desktop_surface *
desktop_surface_at(struct kiwmi_desktop *desktop, double lx, double ly);
struct kiwmi_output *
desktop_surface_get_output(struct kiwmi_desktop_surface *desktop_surface);
void desktop_surface_get_pos(
    struct kiwmi_desktop_surface *desktop_surface,
    int *lx,
    int *ly);

#endif /* KIWMI_DESKTOP_DESKTOP_SURFACE_H */
