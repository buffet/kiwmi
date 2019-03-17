/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "kiwmi/input/input.h"

#include <wayland-server.h>

#include "kiwmi/input.h"
#include "kiwmi/server.h"

bool
input_init(struct kiwmi_input *input)
{
    struct kiwmi_server *server = wl_container_of(input, server, input);

    wl_list_init(&input->keyboards);

    input->new_input.notify = new_input_notify;
    wl_signal_add(&server->backend->events.new_input, &input->new_input);
}
