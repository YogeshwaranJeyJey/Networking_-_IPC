#ifndef CHAT_H
#define CHAT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>

#define MAX_TEXT 100

struct msg {
    long mtype;
    char text[MAX_TEXT];
};

#define MSGKEY 1234

#endif

