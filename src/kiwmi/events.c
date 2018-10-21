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
