#define _POSIX_C_SOURCE 2008009L
#include "shmCommon.h"

static void sleep_tiny(void)
{
    struct timespec ts = {.tv_sec = 0, .tv_nsec = 50 * 1000 * 1000};
    nanosleep(&ts, NULL);
}

int main(void)
{
    if(access(FTOK_PATH, F_OK) != 0) {
        die("Missing key file: run writer first");
    }

    key_t key = ftok(FTOK_PATH, FTOK_PROJID);
    if (key == (key_t)-1) die("ftok");

    int shmid = shmget(key, sizeof(ShmPayload), 0666);
    if (shmid == -1) die("shmget (reader). Run writer first");

    ShmPayload *payload = shmat(shmid, NULL, 0);
    if(payload == (void *)-1) die("shmat");

    printf("[reader] Attached to shmid=%d, waiting for writer...\n", shmid);

    for(;;) {
        int ready = atomic_load_explicit(&payload->ready, memory_order_acquire);
        if(ready == 1) break;
        sleep_tiny();
    }

    atomic_thread_fence(memory_order_acquire);
    printf("[reader] Message: \"%s\" (length=%zu)\n", payload->text, payload->length);

    struct shmid_ds buf;
    if(shmctl(shmid, IPC_STAT, &buf) == -1) die("shmctl (IPC_STAT)");
    printf("[reader] shm info: size=%zu, nattch=%ld\n",
           buf.shm_segsz, buf.shm_nattch);

    if(shmdt(payload) == -1) die("shmdt (reader)");
    printf("[reader] Detached..\n");

    return EXIT_SUCCESS;
}

