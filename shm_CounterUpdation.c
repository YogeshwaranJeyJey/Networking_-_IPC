#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <string.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sched.h>

typedef struct {
    int num_workers;
    _Atomic long counters[];
} ShmPayload;

static void die(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    int num_workers = 4;
    long increments = 100000;
    if (argc >= 2) num_workers = atoi(argv[1]);
    if (argc >= 3) increments = atol(argv[2]);
    if (num_workers <= 0 || increments < 0) {
        fprintf(stderr, "Usage: %s [num_workers>0] [increments>=0]\n", argv[0]);
        return EXIT_FAILURE;
    }
    size_t size = sizeof(ShmPayload) + num_workers * sizeof(_Atomic long);
    int shmid = shmget(IPC_PRIVATE, size, IPC_CREAT | 0600);
    if (shmid == -1) die("shmget");
    ShmPayload *payload = shmat(shmid, NULL, 0);
    if (payload == (void *)-1) die("shmat parent");
    payload->num_workers = num_workers;
    for (int i = 0; i < num_workers; i++) {
        atomic_store(&payload->counters[i], 0);
    }
    pid_t *pids = calloc(num_workers, sizeof(pid_t));
    if (!pids) die("calloc");
    for (int i = 0; i < num_workers; i++) {
        pid_t pid = fork();
        if (pid < 0) die("fork");
        else if (pid == 0) {
            ShmPayload *cp = shmat(shmid, NULL, 0);
            if (cp == (void *)-1) die("shmat child");
            for (long k = 0; k < increments; k++) {
                atomic_fetch_add_explicit(&cp->counters[i], 1, memory_order_relaxed);
                if ((k & 0xFFF) == 0) sched_yield();
            }
            if (shmdt(cp) == -1) die("shmdt child");
            _exit(0);
        } else {
            pids[i] = pid;
        }
    }
    for (int i = 0; i < num_workers; i++) {
        waitpid(pids[i], NULL, 0);
    }
    long total = 0;
    for (int i = 0; i < num_workers; i++) {
        long v = atomic_load_explicit(&payload->counters[i], memory_order_relaxed);
        printf("worker %2d: %ld\n", i, v);
        total += v;
    }
    printf("Total across %d workers: %ld (expected %ld)\n",
           num_workers, total, (long)num_workers * increments);
    if (shmdt(payload) == -1) die("shmdt parent");
    if (shmctl(shmid, IPC_RMID, NULL) == -1) die("shmctl IPC_RMID");
    free(pids);
    return 0;
}