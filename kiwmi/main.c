/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <stdlib.h>

#include <wayland-server.h>
#include <wlr/util/log.h>

#include "kiwmi/server.h"

int
main(void)
{
    wlr_log_init(WLR_DEBUG, NULL);

    struct kiwmi_server server;

    if (!server_init(&server)) {
        wlr_log(WLR_ERROR, "Failed to initialize server");
        exit(EXIT_FAILURE);
    }

    wlr_log(WLR_INFO, "Starting kiwmi v" KIWMI_VERSION);

    if (!server_run(&server)) {
        wlr_log(WLR_ERROR, "Failed to run server");
        exit(EXIT_FAILURE);
    }

    server_fini(&server);
}
