#define _POSIX_C_SOURCE 2008009L
#include<stdio.h>
#include<stdlib.h>
#include<stdatomic.h>
#include<string.h>
#include<errno.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/wait.h>
#include<unistd.h>

typedef struct {
    _Atomic int ready;
    size_t length;
    char text[128];
}ShmPayload;

static void die(const char *message)
{
    perror(message);
    exit(EXIT_FAILURE);
}

int main(void)
{
    //Create anonymous/private segment
    int shmid = shmget(IPC_PRIVATE, sizeof(ShmPayload), IPC_CREAT | 0600);
    if(shmid == -1)
    {
        die("shmget");
    }

    //attach the segment in the parent
    ShmPayload *payload = (ShmPayload*)shmat(shmid, NULL, 0);
    if(payload == (void *)-1)
    {
        die("shmat parent");
    }

    atomic_store(&payload->ready, 0);

    pid_t pid = fork();
    if(pid < 0)
    {
        die("fork");
    }

    if(pid == 0)
    {
        //reader - child process
        ShmPayload *cpointer = (ShmPayload*)shmat(shmid, NULL, 0);
        if(cpointer == (void*)-1)
        {
            die("shmat child");
        }

        while (atomic_load_explicit(&cpointer->ready, memory_order_acquire) == 0)
        {
            usleep(50*1000);
        }

        atomic_thread_fence(memory_order_acquire);
        printf("[child] Got : %s (length = %zu) \n", cpointer->text, cpointer->length);

        if(shmdt(cpointer) == -1)
        {
            die("shmdt child");
        }
        _exit(0);
    }
    else{
        //writer - parent process
        const char *message = "Hello from the parent(writer) via IPC PRIVATE";
        size_t n = strnlen(message, sizeof(payload->text) - 1);
        memcpy(payload->text, message, n);
        payload->text[n] = '\0';
        payload->length = n;
        atomic_thread_fence(memory_order_release);
        atomic_store_explicit(&payload->ready, 1, memory_order_release);

        wait(NULL);

        if(shmdt(payload) == -1)
        {
            die("shmdt parent");
        }

        if(shmctl(shmid, IPC_RMID, NULL) == -1)
        {
            die("shmctl IPC_RMID");
        }
        printf("[parent] Done \n");
    }

    return 0;
}
