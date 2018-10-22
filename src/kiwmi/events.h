/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef EVENTS_H
#define EVENTS_H

#include <xcb/xcb.h>

void handle_xcb_event(xcb_generic_event_t *event);

#endif /* EVENTS_H */
