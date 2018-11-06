/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "xcb.h"

#include <stdlib.h>

#include <xcb/xcb.h>

#include "common.h"

#define ROOT_MASK ( XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT )

static void handle_create_notify_event(xcb_create_notify_event_t *event);
static void handle_destroy_notify_event(xcb_destroy_notify_event_t *event);
static void handle_map_request_event(xcb_map_request_event_t *event);

int g_dpy_fd;
xcb_connection_t *g_dpy;

static xcb_screen_t *g_screen;
static xcb_window_t g_root;

void
init_xcb(void)
{
	uint32_t values[] = { ROOT_MASK };
	xcb_generic_error_t *err;

	g_dpy = xcb_connect(NULL, NULL);

	if (xcb_connection_has_error(g_dpy)) {
		die("failed to open XCB connection\n");
	}

	if (!(g_screen = xcb_setup_roots_iterator(xcb_get_setup(g_dpy)).data)) {
		die("failed to open default display\n");
	}

	g_root = g_screen->root;

	err = xcb_request_check(
		g_dpy,
		xcb_change_window_attributes_checked(
			g_dpy,
			g_root,
			XCB_CW_EVENT_MASK,
			values
		)
	);

	if (err) {
		die("another window manager is already running\n");
	}

	g_dpy_fd = xcb_get_file_descriptor(g_dpy);
}

void
handle_xcb_event(xcb_generic_event_t *event)
{
	switch (event->response_type & ~0x80) {
	case XCB_CREATE_NOTIFY:
		handle_create_notify_event((xcb_create_notify_event_t *)event);
		break;
	case XCB_DESTROY_NOTIFY:
		handle_destroy_notify_event((xcb_destroy_notify_event_t *)event);
		break;
	case XCB_MAP_REQUEST:
		handle_map_request_event((xcb_map_request_event_t *)event);
		break;
	}
}

static void
handle_create_notify_event(xcb_create_notify_event_t *event)
{
	// TODO: handle
}

static void
handle_destroy_notify_event(xcb_destroy_notify_event_t *event)
{
	// TODO: handle
}

static void
handle_map_request_event(xcb_map_request_event_t *event)
{
	// TODO: handle properly
	xcb_map_window(g_dpy, event->window);
}
