#include "chat.h"

int main() {
    int qid;
    struct msg message;

    qid = msgget(MSGKEY, IPC_CREAT | 0666);
    if (qid == -1) {
        perror("msgget");
        exit(1);
    }

    while (1) {
        message.mtype = 1;
        printf("You (A): ");
        fgets(message.text, MAX_TEXT, stdin);
        message.text[strcspn(message.text, "\n")] = '\0';

        if (msgsnd(qid, &message, sizeof(message.text), 0) == -1) {
            perror("msgsnd");
            exit(1);
        }

        if (strcmp(message.text, "exit") == 0)
            break;

        if (msgrcv(qid, &message, sizeof(message.text), 2, 0) == -1) {
            perror("msgrcv");
            exit(1);
        }

        printf("B: %s\n", message.text);

        if (strcmp(message.text, "exit") == 0)
            break;
    }

    msgctl(qid, IPC_RMID, NULL);
    return 0;
}

