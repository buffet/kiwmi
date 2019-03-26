/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef KIWMI_FRONTEND_H
#define KIWMI_FRONTEND_H

#include <stdbool.h>

struct kiwmi_frontend {
    const char *frontend_path;
};

bool frontend_init(struct kiwmi_frontend *frontend, const char *frontend_path);

#endif /* KIWMI_FRONTEND_H */
