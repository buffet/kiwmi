/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

int
main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "Not enough arguments\n");
        exit(EXIT_FAILURE);
    }

    int sock_fd;
    struct sockaddr_un sock_addr;

    memset(&sock_addr, 0, sizeof(sock_addr));

    const char *sock_path = getenv("KIWMI_SOCKET");

    if (!sock_path) {
        fprintf(stderr, "KIWMI_SOCKET not set\n");
        exit(EXIT_FAILURE);
    }

    strncpy(sock_addr.sun_path, sock_path, sizeof(sock_addr.sun_path));

    sock_addr.sun_family = AF_UNIX;

    if ((sock_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "Failed to create socket\n");
        exit(EXIT_FAILURE);
    }

    if (connect(sock_fd, (struct sockaddr *)&sock_addr, sizeof(sock_addr))
        < 0) {
        fprintf(stderr, "Failed to connect to socket\n");
        exit(EXIT_FAILURE);
    }

    FILE *socket_file = fdopen(sock_fd, "r+");

    for (int i = 1; i < argc; ++i) {
        fprintf(socket_file, "%s ", argv[i]);
    }

    fprintf(socket_file, "%c", '\0');
    fflush(socket_file);

    int c;
    while ((c = getc(socket_file)) != EOF) {
        putchar(c);
    }

    fclose(socket_file);
}
