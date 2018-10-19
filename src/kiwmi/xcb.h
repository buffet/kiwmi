/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef XCB_H
#define XCB_H

#include <xcb/xcb.h>

void init_xcb(void);
void handle_xcb_event(xcb_generic_event_t *event);

extern int g_dpy_fd;
extern xcb_connection_t *g_dpy;

#endif /* XCB_H */
