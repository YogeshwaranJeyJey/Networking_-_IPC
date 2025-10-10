#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <errno.h>

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

    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

    flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    struct pollfd fds[2];

    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;

    fds[1].fd = fd;
    fds[1].events = POLLIN;

    printf("Monitoring stdin and file for input using non blocking poll()...\n");

    while (1)
    {
        int ret = poll(fds, 2, 100);

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
            else if(n == -1 && errno != EAGAIN && errno != EWOULDBLOCK){
                perror("Failed Stdin");
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
            else if(n == 0){
                printf("Reached End of file!");
                break;
            }
            else if(n == -1 && errno != EAGAIN && errno != EWOULDBLOCK){
                perror("Failed File!");
            }
        }
    }

    close(fd);
    return 0;
}