/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef KIWMI_DESKTOP_POPUP_H
#define KIWMI_DESKTOP_POPUP_H

struct wlr_xdg_popup;
struct kiwmi_desktop_surface;

struct kiwmi_desktop_surface *
popup_get_desktop_surface(struct wlr_xdg_popup *popup);
void popup_attach(
    struct wlr_xdg_popup *popup,
    struct kiwmi_desktop_surface *desktop_surface);

#endif /* KIWMI_DESKTOP_POPUP_H */
