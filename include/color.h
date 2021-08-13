/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef KIWMI_COLOR_H
#define KIWMI_COLOR_H

#include <stdbool.h>

bool color_parse(const char *hex, float color[static 4]);

#endif /* KIWMI_COLOR_H */
