/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <stdint.h>

#include <wlr/util/log.h>

#include "desktop/stratum.h"
#include "wlr-layer-shell-unstable-v1-protocol.h"

enum kiwmi_stratum
stratum_from_layer_shell_layer(uint32_t layer)
{
    switch (layer) {
    case ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND:
        return KIWMI_STRATUM_LS_BACKGROUND;
    case ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM:
        return KIWMI_STRATUM_LS_BOTTOM;
    case ZWLR_LAYER_SHELL_V1_LAYER_TOP:
        return KIWMI_STRATUM_LS_TOP;
    case ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY:
        return KIWMI_STRATUM_LS_OVERLAY;
    default:
        // We should update our codebase to support the layer
        wlr_log(
            WLR_ERROR, "No matching stratum for layer_shell layer %d", layer);
        return KIWMI_STRATUM_NONE;
    }
}
