/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef KIWMI_DESKTOP_STRATUM_H
#define KIWMI_DESKTOP_STRATUM_H

#include <stdint.h>

/**
 * A stratum is a layer in the scene-graph (the name was chosen to avoid
 * confusion with layer-shell). The root node contains a scene_tree for each
 * stratum, which itself contains a scene_tree per output.
 */

// There are some assumptions made about the values, don't mindlessly change.
enum kiwmi_stratum {
    KIWMI_STRATUM_LS_BACKGROUND, // LS == layer_shell
    KIWMI_STRATUM_LS_BOTTOM,
    KIWMI_STRATUM_NORMAL,
    KIWMI_STRATUM_LS_TOP,
    KIWMI_STRATUM_LS_OVERLAY,
    KIWMI_STRATUM_POPUPS,
    KIWMI_STRATA_COUNT,
    KIWMI_STRATUM_NONE = KIWMI_STRATA_COUNT,
};

enum kiwmi_stratum stratum_from_layer_shell_layer(uint32_t layer);

#endif /* KIWMI_DESKTOP_STRATUM_H */
