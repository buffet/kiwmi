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

#include <unistd.h>

#include <wayland-server.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/util/edges.h>

#include "desktop/desktop_surface.h"

enum kiwmi_view_prop {
    KIWMI_VIEW_PROP_APP_ID,
    KIWMI_VIEW_PROP_TITLE,
};

enum kiwmi_view_type {
    KIWMI_VIEW_XDG_SHELL,
};

struct kiwmi_view {
    struct wl_list link;
    struct kiwmi_desktop_surface desktop_surface;

    struct kiwmi_desktop *desktop;

    const struct kiwmi_view_impl *impl;

    enum kiwmi_view_type type;
    union {
        struct wlr_xdg_surface *xdg_surface;
    };

    struct wlr_surface *wlr_surface;

    struct wlr_box geom;

    struct wl_listener map;
    struct wl_listener unmap;
    struct wl_listener commit;
    struct wl_listener destroy;
    struct wl_listener request_move;
    struct wl_listener request_resize;
    struct wl_listener set_title;

    bool mapped;

    struct {
        struct wl_signal unmap;
        struct wl_signal request_move;
        struct wl_signal request_resize;
        struct wl_signal post_render;
        struct wl_signal pre_render;
        struct wl_signal set_title;
    } events;

    struct kiwmi_xdg_decoration *decoration;
};

struct kiwmi_view_impl {
    void (*close)(struct kiwmi_view *view);
    pid_t (*get_pid)(struct kiwmi_view *view);
    void (*set_activated)(struct kiwmi_view *view, bool activated);
    void (*set_size)(struct kiwmi_view *view, uint32_t width, uint32_t height);
    const char *(
        *get_string_prop)(struct kiwmi_view *view, enum kiwmi_view_prop prop);
    void (*set_tiled)(struct kiwmi_view *view, enum wlr_edges edges);
};

struct kiwmi_request_resize_event {
    struct kiwmi_view *view;
    uint32_t edges;
};

void view_close(struct kiwmi_view *view);
pid_t view_get_pid(struct kiwmi_view *view);
void view_get_size(struct kiwmi_view *view, uint32_t *width, uint32_t *height);
const char *view_get_app_id(struct kiwmi_view *view);
const char *view_get_title(struct kiwmi_view *view);
void view_set_activated(struct kiwmi_view *view, bool activated);
void view_set_size(struct kiwmi_view *view, uint32_t width, uint32_t height);
void view_set_pos(struct kiwmi_view *view, uint32_t x, uint32_t y);
void view_set_tiled(struct kiwmi_view *view, enum wlr_edges edges);
void view_set_hidden(struct kiwmi_view *view, bool hidden);

void view_focus(struct kiwmi_view *view);
struct kiwmi_view *view_at(struct kiwmi_desktop *desktop, double lx, double ly);
void view_move(struct kiwmi_view *view);
void view_resize(struct kiwmi_view *view, uint32_t edges);
struct kiwmi_view *view_create(
    struct kiwmi_desktop *desktop,
    enum kiwmi_view_type type,
    const struct kiwmi_view_impl *impl);

#endif /* KIWMI_DESKTOP_VIEW_H */
