/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "kiwmi/frontend.h"

#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include <wlr/util/log.h>

static bool
spawn_frontend(const char *path)
{
    pid_t pid = fork();

    if (pid < 0) {
        wlr_log(WLR_ERROR, "Failed to start frontend (fork)");
        return false;
    }

    if (pid == 0) {
        execlp(path, path, NULL);
        wlr_log(WLR_ERROR, "Failed to start frontend (exec), continuing");
        _exit(EXIT_FAILURE);
    }

    return true;
}

bool
frontend_init(struct kiwmi_frontend *frontend, const char *frontend_path)
{
    frontend->frontend_path = frontend_path;

    if (strcmp(frontend_path, "NONE") == 0) {
        wlr_log(WLR_ERROR, "Launching without a frontend");
        return true;
    } else {
        return spawn_frontend(frontend_path);
    }
}
