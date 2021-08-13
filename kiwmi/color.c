/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "color.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

bool
color_parse(const char *hex, float color[static 4])
{
    if (hex[0] == '#') {
        ++hex;
    }

    int len = strlen(hex);
    if (len != 6 && len != 8) {
        return false;
    }

    uint32_t rgba = (uint32_t)strtoul(hex, NULL, 16);
    if (len == 6) {
        rgba = (rgba << 8) | 0xff;
    }

    for (size_t i = 0; i < 4; ++i) {
        color[3 - i] = (rgba & 0xff) / 255.0;
        rgba >>= 8;
    }

    // premultiply alpha
    color[0] *= color[3];
    color[1] *= color[3];
    color[2] *= color[3];

    return true;
}
