#define _POSIX_C_SOURCE 2008009L
#include "shmCommon.h"

int main(int argc, char **argv)
{
    if(argc < 2) {
        fprintf(stderr, "Usage: %s \"message to publish\"\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Ensure key file exists
    if(access(FTOK_PATH, F_OK) != 0) {
        FILE *file = fopen(FTOK_PATH, "ab");
        if(!file) die("fopen key file");
        fclose(file);
    }

    // Generate System V IPC key
    key_t key = ftok(FTOK_PATH, FTOK_PROJID);
    if (key == (key_t)-1) die("ftok");

    // Create/Get shared memory
    int shmflag = IPC_CREAT | 0666;
    int shmid = shmget(key, sizeof(ShmPayload), shmflag);
    if (shmid == -1) die("shmget");

    // Attach to memory
    ShmPayload *payload = shmat(shmid, NULL, 0);
    if(payload == (void *)-1) die("shmat");

    // Initialize and write data
    atomic_store_explicit(&payload->ready, 0, memory_order_relaxed);

    const char *message = argv[1];
    size_t n = strnlen(message, SHM_TEXT_CAP - 1);
    memcpy(payload->text, message, n);
    payload->text[n] = '\0';
    payload->length = n;

    // Publish message
    atomic_thread_fence(memory_order_release);
    atomic_store_explicit(&payload->ready, 1, memory_order_release);

    printf("[writer] shmid=%d key=0x%lx wrote \"%s\" (length=%zu) ready=1\n",
           shmid, (unsigned long)key, payload->text, payload->length);
    printf("[writer] You can now run the reader...\n");

    sleep(5); // keep alive so reader can attach

    // Detach
    if(shmdt(payload) == -1) die("shmdt (writer)");
    printf("[writer] Detached..\n");

    // Remove shared memory
    if(shmctl(shmid, IPC_RMID, NULL) == -1) die("shmctl (IPC_RMID)");
    printf("[writer] Shared memory removed.\n");

    return EXIT_SUCCESS;
}

