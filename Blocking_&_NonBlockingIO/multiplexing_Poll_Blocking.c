#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

int main(void)
{
    int fd;
    char buffer[100];

    fd = open("test.txt", O_RDONLY);
    if (fd < 0)
    {
        perror("open");
        return 1;
    }

    struct pollfd fds[2];

    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;

    fds[1].fd = fd;
    fds[1].events = POLLIN;

    printf("Monitoring stdin and file for input using blocking poll()...\n");

    while (1)
    {
        int ret = poll(fds, 2, -1);

        if (ret < 0)
        {
            perror("poll");
            break;
        }

        if (fds[0].revents & POLLIN)
        {
            int n = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);
            if (n > 0)
            {
                buffer[n] = '\0';
                printf("You typed: %s\n", buffer);
            }
        }

        if (fds[1].revents & POLLIN)
        {
            int n = read(fd, buffer, sizeof(buffer) - 1);
            if (n > 0)
            {
                buffer[n] = '\0';
                printf("File content: %s\n", buffer);
            }
        }
    }

    close(fd);
    return 0;
}