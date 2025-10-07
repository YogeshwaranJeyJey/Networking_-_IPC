#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SOCKET_PATH "/tmp/kvstore.sock"
#define BUF_SIZE 1024

int main(void) {
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket");
        exit(1);
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path)-1);

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect");
        exit(1);
    }

    printf("[client] connected to server\n");
    printf("Enter commands (SET key value | GET key | EXIT)\n");

    char buf[BUF_SIZE];
    while (fgets(buf, sizeof(buf), stdin)) {
        if (write(fd, buf, strlen(buf)) < 0) {
            perror("write");
            break;
        }

        char reply[BUF_SIZE];
        ssize_t n = read(fd, reply, sizeof(reply)-1);
        if (n <= 0) break;
        reply[n] = '\0';
        printf("[server reply] %s", reply);

        if (strncmp(buf, "EXIT", 4) == 0) break;
    }

    close(fd);
    return 0;
}
