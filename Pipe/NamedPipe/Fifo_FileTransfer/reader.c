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
        fprintf(stderr, "Usage: %s <output_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (mkfifo(FIFO_PATH, 0666) == -1) {
        perror("mkfifo");
    }
    
    int fifo_fd = open(FIFO_PATH, O_RDONLY);
    if (fifo_fd < 0) {
        perror("open fifo");
        exit(EXIT_FAILURE);
    }

    int out_fd = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (out_fd < 0) {
        perror("open output file");
        close(fifo_fd);
        exit(EXIT_FAILURE);
    }

    char buf[BUF_SIZE];
    ssize_t n;
    while ((n = read(fifo_fd, buf, sizeof(buf))) > 0) {
        if (write(out_fd, buf, n) != n) {
            perror("write to output file");
            break;
        }
    }

    close(out_fd);
    close(fifo_fd);

    printf("[reader] File written to: %s\n", argv[1]);
    return 0;
}
