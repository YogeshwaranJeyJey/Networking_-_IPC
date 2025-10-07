#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main(){
    int fd = open("myFifo", O_WRONLY);
    if(fd == -1){
        perror("Error while opening the Fifo file!\n");
        return 1;
    }
    char write_msg[] = "Hello from writer!";
    write(fd, write_msg, sizeof(write_msg));
    printf("Writer wrote: %s\n", write_msg);
    close(fd);
    return 0;
}