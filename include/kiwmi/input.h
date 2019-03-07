/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef KIWMI_INPUT_H
#define KIWMI_INPUT_H

#include <wayland-server.h>

void new_input_notify(struct wl_listener *listener, void *data);

#endif /* KIWMI_INPUT_H */
