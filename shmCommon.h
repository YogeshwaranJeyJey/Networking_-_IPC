#define _POSIX_C_SOURCE 2008009L
#include<stdio.h>
#include<stdlib.h>
#include<stdatomic.h>
#include<string.h>
#include<errno.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<time.h>
#include<unistd.h>

#define SHM_TEXT_CAP 256
#define FTOK_PATH "./shm_demo_keyfile"
#define FTOK_PROJID 0x42

typedef struct{
    _Atomic int ready;
    size_t length;
    char text[SHM_TEXT_CAP];
}ShmPayload;


static void die(const char *message)
{
    perror(message);
    exit(EXIT_FAILURE);
}
