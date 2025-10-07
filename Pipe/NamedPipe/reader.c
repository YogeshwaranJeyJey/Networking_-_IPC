#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main(){
    int fd = open("myFifo", O_RDONLY);
    if(fd == -1){
        perror("Error while opening the Fifo file!\n");
        return 1;
    }
    char buffer[100];
    read(fd, buffer, sizeof(buffer));
    printf("Reader read: %s\n", buffer);
    close(fd);
    return 0;
}