#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

#define FIFO_PATH "/tmp/myfifo"
#define BUF_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (mkfifo(FIFO_PATH, 0666) == -1) {
        perror("mkfifo");
    }

    int in_fd = open(argv[1], O_RDONLY);
    if (in_fd < 0) {
        perror("open input file");
        exit(EXIT_FAILURE);
    }

    int fifo_fd = open(FIFO_PATH, O_WRONLY);
    if (fifo_fd < 0) {
        perror("open fifo");
        close(in_fd);
        exit(EXIT_FAILURE);
    }

    char buf[BUF_SIZE];
    ssize_t n;
    while ((n = read(in_fd, buf, sizeof(buf))) > 0) {
        if (write(fifo_fd, buf, n) != n) {
            perror("write to fifo");
            break;
        }
    }

    close(in_fd);
    close(fifo_fd);

    printf("[writer] Finished sending file: %s\n", argv[1]);
    return 0;
}
