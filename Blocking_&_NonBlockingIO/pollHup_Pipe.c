#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <fcntl.h>
#include <sys/stat.h>

int main(){
    if(mkfifo("myFifo", 0666) == -1){
        perror("Fifo create");
        exit(EXIT_FAILURE);
    }

    int pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if(pid == 0){
        int fd = open("myFifo", O_WRONLY);
        if (fd < 0) {
            perror("Child: open");
            exit(EXIT_FAILURE);
        }
        char msg[] = "Hello! This is child process";
        write(fd, msg, sizeof(msg));
        sleep(1);
        close(fd);
        return 0;
    }
    else{
       int fd = open("myFifo", O_RDONLY | O_NONBLOCK);
        if (fd < 0) {
            perror("Parent: open");
            return 1;
        }
        
        struct pollfd pfd;
        pfd.fd = fd;
        pfd.events = POLLIN;

        char buffer[100];
        while(1){
            int ret = poll(&pfd, 1, 500);
            if(ret < 0){
                perror("Poll");
                exit(EXIT_FAILURE);
            }
            if (pfd.revents & POLLIN){
                int n = read(pfd.fd, buffer, sizeof(buffer) - 1);
                if (n > 0){
                    buffer[n] = '\0';
                    printf("The child wrote: %s\n", buffer);
                }
            }
            if (pfd.revents & POLLHUP) {
                printf("POLLHUP detected! Child is closed.\n");
                break;
            }
        }
        close(fd);
        unlink("myFifo");
    }
    return 0;
}