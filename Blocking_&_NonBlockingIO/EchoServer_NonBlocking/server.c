#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/echo_socket"
#define MAX_CLIENTS FD_SETSIZE
#define BUFFER_SIZE 1024

void set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl(F_GETFL)");
        exit(EXIT_FAILURE);
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl(F_SETFL)");
        exit(EXIT_FAILURE);
    }
}

int main() {
    int listener, newfd, maxfd, i;
    struct sockaddr_un server_addr, client_addr;
    socklen_t addrlen;
    char buffer[BUFFER_SIZE];

    unlink(SOCKET_PATH);

    if ((listener = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    if (bind(listener, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(listener, 10) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    set_nonblocking(listener);

    int client_sockets[MAX_CLIENTS];
    for (i = 0; i < MAX_CLIENTS; i++) client_sockets[i] = -1;

    printf("Server listening on %s\n", SOCKET_PATH);

    while (1) {
        fd_set readfds;
        FD_ZERO(&readfds);

        FD_SET(listener, &readfds);
        maxfd = listener;

        for (i = 0; i < MAX_CLIENTS; i++) {
            int fd = client_sockets[i];
            if (fd > 0) {
                FD_SET(fd, &readfds);
                if (fd > maxfd) maxfd = fd;
            }
        }

        int activity = select(maxfd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0 && errno != EINTR) {
            perror("select");
            break;
        }

        if (FD_ISSET(listener, &readfds)) {
            addrlen = sizeof(client_addr);
            newfd = accept(listener, (struct sockaddr *)&client_addr, &addrlen);
            if (newfd == -1) {
                if (errno != EWOULDBLOCK && errno != EAGAIN)
                    perror("accept");
            } else {
                set_nonblocking(newfd);
                printf("New client connected: fd=%d\n", newfd);

                for (i = 0; i < MAX_CLIENTS; i++) {
                    if (client_sockets[i] == -1) {
                        client_sockets[i] = newfd;
                        break;
                    }
                }
                if (i == MAX_CLIENTS) {
                    printf("Too many clients. Closing fd=%d\n", newfd);
                    close(newfd);
                }
            }
        }

        for (i = 0; i < MAX_CLIENTS; i++) {
            int fd = client_sockets[i];
            if (fd > 0 && FD_ISSET(fd, &readfds)) {
                while (1) {
                    int nbytes = recv(fd, buffer, sizeof(buffer) - 1, 0);
                    if (nbytes > 0) {
                        buffer[nbytes] = '\0';
                        printf("Client fd=%d says: %s", fd, buffer);
                    } else if (nbytes == 0) {
                        printf("Client disconnected: fd=%d\n", fd);
                        close(fd);
                        client_sockets[i] = -1;
                        break;
                    } else {
                        if (errno == EWOULDBLOCK || errno == EAGAIN) {
                            // No more data for now
                            break;
                        } else {
                            perror("recv");
                            close(fd);
                            client_sockets[i] = -1;
                            break;
                        }
                    }
                }
            }
        }
    }

    close(listener);
    unlink(SOCKET_PATH);
    return 0;
}
