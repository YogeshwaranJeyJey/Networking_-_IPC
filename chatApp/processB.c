#include "chat.h"

int main() {
    int qid;
    struct msg message;

    qid = msgget(MSGKEY, 0666);
    if (qid == -1) {
        perror("msgget");
        exit(1);
    }

    while (1) {
        if (msgrcv(qid, &message, sizeof(message.text), 1, 0) == -1) {
            perror("msgrcv");
            exit(1);
        }

        printf("A: %s\n", message.text);

        if (strcmp(message.text, "exit") == 0)
            break;

        message.mtype = 2;
        printf("You (B): ");
        fgets(message.text, MAX_TEXT, stdin);
        message.text[strcspn(message.text, "\n")] = '\0';

        if (msgsnd(qid, &message, sizeof(message.text), 0) == -1) {
            perror("msgsnd");
            exit(1);
        }

        if (strcmp(message.text, "exit") == 0)
            break;
    }

    return 0;
}

