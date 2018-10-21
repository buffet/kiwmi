#ifndef EVENTS_H
#define EVENTS_H

#include <xcb/xcb.h>

void handle_xcb_event(xcb_generic_event_t *event);

#endif /* EVENTS_H */
