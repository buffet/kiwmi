/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "events.h"

#include <stdlib.h>

#include <xcb/xcb.h>

void
handle_xcb_event(xcb_generic_event_t *event)
{
	switch (event->response_type) {
	case XCB_CREATE_NOTIFY:
		// TODO: insert into tree
		break;
	case XCB_DESTROY_NOTIFY:
		// TODO: destroy window
		break;
	}

	free(event);
}
