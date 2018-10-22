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
