#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <poll.h>

#define SOCK_PATH "/tmp/pollhup_demoSock"

int main(void) {
    int srv_sock, cli_sock;
    struct sockaddr_un addr;
    socklen_t addrlen;

    unlink(SOCK_PATH);

    srv_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (srv_sock < 0) { 
        perror("socket"); 
        exit(1); 
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCK_PATH, sizeof(addr.sun_path) - 1);

    if (bind(srv_sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind"); exit(1);
    }

    if (listen(srv_sock, 5) < 0) {
        perror("listen"); 
        exit(1); 
    }

    printf("Server listening on %s\n", SOCK_PATH);

    pid_t pid = fork();
    if (pid == 0) {
        sleep(1);
        int c = socket(AF_UNIX, SOCK_STREAM, 0);
        if (c < 0) {
            perror("client socket"); 
            exit(1); 
        }

        struct sockaddr_un caddr;
        memset(&caddr, 0, sizeof(caddr));
        caddr.sun_family = AF_UNIX;
        strncpy(caddr.sun_path, SOCK_PATH, sizeof(caddr.sun_path) - 1);

        if (connect(c, (struct sockaddr*)&caddr, sizeof(caddr)) < 0) {
            perror("client connect"); 
            exit(1);
        }

        char *msg = "Hello from client";
        write(c, msg, strlen(msg));
        printf("Client sent message and will close\n");
        close(c);
        exit(0);
    }

    addrlen = sizeof(addr);
    cli_sock = accept(srv_sock, (struct sockaddr*)&addr, &addrlen);
    if (cli_sock < 0) { 
        perror("accept"); 
        exit(1); 
    }
    printf("Server accepted client\n");

    struct pollfd pfd;
    pfd.fd = cli_sock;
    pfd.events = POLLIN | POLLHUP;

    while (1) {
        int ret = poll(&pfd, 1, -1);
        if (ret < 0) { 
            perror("poll"); 
            break; 
        }

        if (pfd.revents & POLLIN) {
            char buf[100];
            ssize_t n = read(cli_sock, buf, sizeof(buf)-1);
            if (n > 0) {
                buf[n] = '\0';
                printf("Server received: %s\n", buf);
            } else if (n == 0) {
                printf("Server: client closed\n");
                break;
            }
        }
        if (pfd.revents & POLLHUP) {
            printf("Server detected POLLHUP (client disconnected)\n");
            break;
        }
    }

    close(cli_sock);
    close(srv_sock);
    unlink(SOCK_PATH);
    return 0;
}
