#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/select.h>

int main(){
    int fd;
    char buffer[100];
    fd = open("test.txt", O_RDONLY);
    if(fd < 0){
        perror("open");
        exit(EXIT_FAILURE);
    }
    fd_set readFds;
    int maxFd = (fd > STDIN_FILENO ? fd : STDIN_FILENO) + 1;
    printf("Monitoring stdin and file for input in blocking mode...\n");
    while(1){
        FD_ZERO(&readFds);
        FD_SET(STDIN_FILENO, &readFds);
        FD_SET(fd, &readFds);

        int activity = select(maxFd, &readFds, NULL, NULL, NULL);
        if(activity < 0){
            perror("Select");
            exit(EXIT_FAILURE);
        }

        if(FD_ISSET(STDIN_FILENO, &readFds)){
            int n = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);
            if(n > 0){
                buffer[n] = '\0';
                printf("You typed: %s", buffer);
            }
        }

        if(FD_ISSET(fd, &readFds)){
            int n = read(fd, buffer, sizeof(buffer) - 1);
            if(n > 0){
                buffer[n] = '\0';
                printf("File content: %s", buffer);
            }
        }
    }
    close(fd);
    return 0;
}