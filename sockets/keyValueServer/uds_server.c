#define _GNU_SOURCE
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define SOCKET_PATH "/tmp/kvstore.sock"
#define BACKLOG 5
#define BUF_SIZE 1024
#define MAX_ITEMS 100

struct kvpair {
    char key[64];
    char value[256];
};
static struct kvpair store[MAX_ITEMS];
static int store_count = 0;

static int listen_fd = -1;

void cleanup(void) {
    if (listen_fd != -1)
        close(listen_fd);
    unlink(SOCKET_PATH);
}

void die(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

char* kv_get(const char *key) {
    for (int i = 0; i < store_count; i++) {
        if (strcmp(store[i].key, key) == 0)
            return store[i].value;
    }
    return NULL;
}

void kv_set(const char *key, const char *value) {
    for (int i = 0; i < store_count; i++) {
        if (strcmp(store[i].key, key) == 0) {
            strncpy(store[i].value, value, sizeof(store[i].value)-1);
            store[i].value[sizeof(store[i].value)-1] = '\0';
            return;
        }
    }
    if (store_count < MAX_ITEMS) {
        strncpy(store[store_count].key, key, sizeof(store[store_count].key)-1);
        strncpy(store[store_count].value, value, sizeof(store[store_count].value)-1);
        store[store_count].key[sizeof(store[store_count].key)-1] = '\0';
        store[store_count].value[sizeof(store[store_count].value)-1] = '\0';
        store_count++;
    }
}

int main(void) {
    atexit(cleanup);

    listen_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (listen_fd < 0) die("socket");

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path)-1);

    unlink(SOCKET_PATH);
    if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        die("bind");

    if (chmod(SOCKET_PATH, 0666) < 0)
        die("chmod");

    if (listen(listen_fd, BACKLOG) < 0)
        die("listen");

    printf("[server] listening on %s\n", SOCKET_PATH);

    for (;;) {
        int client_fd = accept(listen_fd, NULL, NULL);
        if (client_fd < 0) {
            if (errno == EINTR) continue;
            die("accept");
        }

        printf("[server] client connected\n");

        char buf[BUF_SIZE];
        ssize_t n;
        while ((n = read(client_fd, buf, sizeof(buf)-1)) > 0) {
            buf[n] = '\0';

            char cmd[16], key[64], value[256];
            if (sscanf(buf, "SET %63s %255[^\n]", key, value) == 2) {
                kv_set(key, value);
                dprintf(client_fd, "OK\n");
            } else if (sscanf(buf, "GET %63s", key) == 1) {
                char *val = kv_get(key);
                if (val)
                    dprintf(client_fd, "%s\n", val);
                else
                    dprintf(client_fd, "NOT_FOUND\n");
            } else if (strncmp(buf, "EXIT", 4) == 0) {
                dprintf(client_fd, "BYE\n");
                break;
            } else {
                dprintf(client_fd, "ERR unknown command\n");
            }
        }
        close(client_fd);
        printf("[server] client disconnected\n");
    }
}
