/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef KIWMI_DESKTOP_VIEW_H
#define KIWMI_DESKTOP_VIEW_H

#include <stdbool.h>
#include <stdlib.h>

#include <wayland-server.h>
#include <wlr/types/wlr_xdg_shell.h>

enum kiwmi_view_type {
    KIWMI_VIEW_XDG_SHELL,
};

struct kiwmi_view {
    struct wl_list link;

    struct kiwmi_desktop *desktop;

    const struct kiwmi_view_impl *impl;

    enum kiwmi_view_type type;
    union {
        struct wlr_xdg_surface *xdg_surface;
    };

    struct wlr_surface *wlr_surface;

    struct wl_listener map;
    struct wl_listener unmap;
    struct wl_listener destroy;

    double x;
    double y;

    bool mapped;
    bool hidden;

    struct {
        struct wl_signal unmap;
    } events;
};

struct kiwmi_view_impl {
    void (*close)(struct kiwmi_view *view);
    void (*for_each_surface)(
        struct kiwmi_view *view,
        wlr_surface_iterator_func_t iterator,
        void *user_data);
    void (*resize)(struct kiwmi_view *view, uint32_t width, uint32_t height);
    void (*set_activated)(struct kiwmi_view *view, bool activated);
    void (*set_tiled)(struct kiwmi_view *view, bool tiled);
    struct wlr_surface *(*surface_at)(
        struct kiwmi_view *view,
        double sx,
        double sy,
        double *sub_x,
        double *sub_y);
};

void view_close(struct kiwmi_view *view);
void view_for_each_surface(
    struct kiwmi_view *view,
    wlr_surface_iterator_func_t iterator,
    void *user_data);
void view_resize(struct kiwmi_view *view, uint32_t width, uint32_t height);
void view_set_activated(struct kiwmi_view *view, bool activated);
void view_set_tiled(struct kiwmi_view *view, bool tiled);
struct wlr_surface *view_surface_at(
    struct kiwmi_view *view,
    double sx,
    double sy,
    double *sub_x,
    double *sub_y);

void view_focus(struct kiwmi_view *view);
struct kiwmi_view *view_at(
    struct kiwmi_desktop *desktop,
    double lx,
    double ly,
    struct wlr_surface **surface,
    double *sx,
    double *sy);
struct kiwmi_view *view_create(
    struct kiwmi_desktop *desktop,
    enum kiwmi_view_type type,
    const struct kiwmi_view_impl *impl);

#endif /* KIWMI_DESKTOP_VIEW_H */
