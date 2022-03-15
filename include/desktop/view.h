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

enum kiwmi_view_prop {
    KIWMI_VIEW_PROP_APP_ID,
    KIWMI_VIEW_PROP_TITLE,
};

enum kiwmi_view_type {
    KIWMI_VIEW_XDG_SHELL,
};

struct kiwmi_view {
    struct wl_list link;
    struct wl_list children; // struct kiwmi_view_child::link

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
    struct wl_listener new_popup;
    struct wl_listener new_subsurface;
    struct wl_listener request_move;
    struct wl_listener request_resize;

    int x;
    int y;

    bool mapped;
    bool hidden;

    struct {
        struct wl_signal unmap;
        struct wl_signal request_move;
        struct wl_signal request_resize;
        struct wl_signal post_render;
        struct wl_signal pre_render;
    } events;

    struct kiwmi_xdg_decoration *decoration;
};

struct kiwmi_view_impl {
    void (*close)(struct kiwmi_view *view);
    void (*for_each_surface)(
        struct kiwmi_view *view,
        wlr_surface_iterator_func_t callback,
        void *user_data);
    pid_t (*get_pid)(struct kiwmi_view *view);
    void (*set_activated)(struct kiwmi_view *view, bool activated);
    void (*set_size)(struct kiwmi_view *view, uint32_t width, uint32_t height);
    const char *(
        *get_string_prop)(struct kiwmi_view *view, enum kiwmi_view_prop prop);
    void (*set_tiled)(struct kiwmi_view *view, enum wlr_edges edges);
    struct wlr_surface *(*surface_at)(
        struct kiwmi_view *view,
        double sx,
        double sy,
        double *sub_x,
        double *sub_y);
};

enum kiwmi_view_child_type {
    KIWMI_VIEW_CHILD_SUBSURFACE,
    KIWMI_VIEW_CHILD_XDG_POPUP,
};

struct kiwmi_view_child {
    struct wl_list link;
    struct wl_list children; // struct kiwmi_view_child::link

    struct kiwmi_view *view;
    struct kiwmi_view_child *parent;

    enum kiwmi_view_child_type type;
    const struct kiwmi_view_child_impl *impl;

    struct wlr_surface *wlr_surface;
    union {
        struct wlr_subsurface *wlr_subsurface;
        struct wlr_xdg_popup *wlr_xdg_popup;
    };

    bool mapped;

    struct wl_listener commit;
    struct wl_listener map;
    struct wl_listener unmap;
    struct wl_listener new_popup;
    struct wl_listener new_subsurface;
    struct wl_listener extension_destroy; // the union'ed object destroy
    struct wl_listener surface_destroy;   // wlr_surface::events.destroy
};

struct kiwmi_view_child_impl {
    void (*reconfigure)(struct kiwmi_view_child *child);
};

struct kiwmi_request_resize_event {
    struct kiwmi_view *view;
    uint32_t edges;
};

void view_close(struct kiwmi_view *view);
void view_for_each_surface(
    struct kiwmi_view *view,
    wlr_surface_iterator_func_t callback,
    void *user_data);
pid_t view_get_pid(struct kiwmi_view *view);
void view_get_size(struct kiwmi_view *view, uint32_t *width, uint32_t *height);
const char *view_get_app_id(struct kiwmi_view *view);
const char *view_get_title(struct kiwmi_view *view);
void view_set_activated(struct kiwmi_view *view, bool activated);
void view_set_size(struct kiwmi_view *view, uint32_t width, uint32_t height);
void view_set_pos(struct kiwmi_view *view, uint32_t x, uint32_t y);
void view_set_tiled(struct kiwmi_view *view, enum wlr_edges edges);
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
void view_move(struct kiwmi_view *view);
void view_resize(struct kiwmi_view *view, uint32_t edges);
struct kiwmi_view *view_create(
    struct kiwmi_desktop *desktop,
    enum kiwmi_view_type type,
    const struct kiwmi_view_impl *impl);

void
view_init_subsurfaces(struct kiwmi_view_child *child, struct kiwmi_view *view);
bool view_child_is_mapped(struct kiwmi_view_child *child);
void view_child_damage(struct kiwmi_view_child *child);
void view_child_destroy(struct kiwmi_view_child *child);
struct kiwmi_view_child *view_child_create(
    struct kiwmi_view_child *parent,
    struct kiwmi_view *view,
    struct wlr_surface *wlr_surface,
    enum kiwmi_view_child_type type,
    const struct kiwmi_view_child_impl *impl);
struct kiwmi_view_child *view_child_subsurface_create(
    struct kiwmi_view_child *parent,
    struct kiwmi_view *view,
    struct wlr_subsurface *subsurface);

#endif /* KIWMI_DESKTOP_VIEW_H */
