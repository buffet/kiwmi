/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef COMMON_H
#define COMMON_H

#include <stdnoreturn.h>

#define SOCK_ENV_VAR  "KIWMI_SOCKET"
#define SOCK_DEF_PATH "/tmp/kiwmi.sock"

#define CONFIG_FILE "kiwmi/kiwmirc"

noreturn void die(char *fmt, ...);
void warn(char *fmt, ...);

extern const char *argv0;

#endif /* COMMON_H */
