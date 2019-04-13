/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "kiwmi/frontend.h"

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include <wlr/util/log.h>

#include "kiwmi/commands.h"
#include "kiwmi/server.h"

static void
display_destroy_notify(struct wl_listener *listener, void *UNUSED(data))
{
    struct kiwmi_frontend *frontend =
        wl_container_of(listener, frontend, display_destroy);

    if (frontend->sock_event_source) {
        wl_event_source_remove(frontend->sock_event_source);
    }

    close(frontend->sock_fd);
    unlink(frontend->sock_path);
    free(frontend->sock_path);

    wl_list_remove(&frontend->display_destroy.link);
}

static int
ipc_connection(int fd, uint32_t mask, void *data)
{
    wlr_log(WLR_DEBUG, "Received an IPC event");

    struct kiwmi_server *server = data;

    if (!(mask & WL_EVENT_READABLE)) {
        return 0;
    }

    int client_fd = accept(fd, NULL, NULL);

    if (client_fd < 0) {
        wlr_log(WLR_ERROR, "Failed to accept client connection");
        return 0;
    }

    FILE *client_file = fdopen(client_fd, "r+");

    size_t buffer_size = BUFSIZ;
    size_t msg_len     = 0;
    char *msg          = malloc(buffer_size);
    if (!msg) {
        wlr_log(WLR_ERROR, "Failed to allocate memory");
        fclose(client_file);
        return 0;
    }

    int c;
    while ((c = getc(client_file)) != '\0') {
        if (msg_len >= buffer_size) {
            buffer_size *= 2;
            char *tmp = realloc(msg, buffer_size);
            if (!tmp) {
                wlr_log(WLR_ERROR, "Failed to allocate memory");
                fclose(client_file);
                free(msg);
                return 0;
            }
            msg = tmp;
        }

        msg[msg_len++] = c;
    }

    msg[msg_len] = '\0';

    for (char *cmd = strtok(msg, "\n"); cmd; cmd = strtok(NULL, "\n")) {
        if (!handle_client_command(cmd, client_file, server)) {
            break;
        }
    }

    fclose(client_file);
    free(msg);

    return 0;
}

static bool
ipc_init(struct kiwmi_frontend *frontend)
{
    struct kiwmi_server *server = wl_container_of(frontend, server, frontend);
    struct sockaddr_un sock_addr;

    memset(&sock_addr, 0, sizeof(sock_addr));

    size_t path_len = snprintf(
        sock_addr.sun_path,
        sizeof(sock_addr.sun_path),
        "%s/kiwmi_%" PRIdMAX ".sock",
        getenv("XDG_RUNTIME_DIR"),
        (intmax_t)getpid());

    frontend->sock_path = malloc(path_len + 1);
    if (!frontend->sock_path) {
        wlr_log(WLR_ERROR, "Failed to allocate memory");
        return false;
    }

    strcpy(frontend->sock_path, sock_addr.sun_path);

    setenv("KIWMI_SOCKET", sock_addr.sun_path, true);

    sock_addr.sun_family = AF_UNIX;

    if ((frontend->sock_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        wlr_log(WLR_ERROR, "Failed to create socket");
        return false;
    }

    if (fcntl(frontend->sock_fd, F_SETFD, FD_CLOEXEC) < 0) {
        wlr_log(WLR_ERROR, "Failed to set CLOEXEC on socket");
        return false;
    }

    unlink(sock_addr.sun_path);

    if (bind(
            frontend->sock_fd, (struct sockaddr *)&sock_addr, sizeof(sock_addr))
        < 0) {
        wlr_log(WLR_ERROR, "Failed to bind socket");
        return false;
    }

    if (listen(frontend->sock_fd, 3) < 0) {
        wlr_log(WLR_ERROR, "Failed to listen to socket");
        return false;
    }

    frontend->display_destroy.notify = display_destroy_notify;
    wl_display_add_destroy_listener(
        server->wl_display, &frontend->display_destroy);

    frontend->sock_event_source = wl_event_loop_add_fd(
        server->wl_event_loop,
        frontend->sock_fd,
        WL_EVENT_READABLE,
        ipc_connection,
        server);

    return true;
}

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

    if (!ipc_init(frontend)) {
        wlr_log(WLR_ERROR, "Failed to create socket");
        return false;
    }

    if (strcmp(frontend_path, "NONE") == 0) {
        wlr_log(WLR_ERROR, "Launching without a frontend");
        return true;
    } else {
        return spawn_frontend(frontend_path);
    }
}
