
#include <stdio.h>
#include <sys/stat.h>

int main(){
	char* fifoPath = "myFifo";
	if(mkfifo(fifoPath, 0666) == -1){
		perror("Pipe creation failed!\n");
		return 1;
	}
	printf("Pipe created successfully!\n");
	return 0;
}
