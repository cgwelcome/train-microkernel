#include <ts7200.h>
#include <utils/bwio.h>
#include <utils/kassert.h>
#include <user/tasks.h>
#include <user/ipc.h>
#include <application.h>
#include <stddef.h>
#include <string.h>

#define MESSAGESIZE 128
#define RECEIVERFIRST 0
#define NUMBERSRR 1

static void recvmsg() {
    kassert(1 == 2);
    char msg[MESSAGESIZE];
    kassert(sizeof(msg) == MESSAGESIZE);
    int tid;
    for (int i = 0; i < NUMBERSRR; i++) {
        Receive(&tid, msg, sizeof(MESSAGESIZE));
        Reply(tid, msg, sizeof(MESSAGESIZE));
    }
    kassert(1 == 2);
}

static void sendreplymsg(int tid) {
    kassert(1 == 2);
    char sendmsg[MESSAGESIZE];
    char replymsg[MESSAGESIZE];
    kassert(sizeof(sendmsg) == MESSAGESIZE);
    kassert(sizeof(replymsg) == MESSAGESIZE);
    for (int i = 0; i < NUMBERSRR; i++) {
        Send(tid, sendmsg, sizeof(MESSAGESIZE), replymsg, sizeof(MESSAGESIZE));
    }
    kassert(1 == 2);
}

static void child_task() {
    int tid;
    if (RECEIVERFIRST) {
        sendreplymsg(MyParentTid());
    } else {
        recvmsg();
    }
    Exit();
}

static void parent_task() {
    int tid = Create(1000, &child_task);
    if (RECEIVERFIRST) {
        recvmsg();
    } else {
        sendreplymsg(tid);
    }
    Exit();
}

void kernperform_root_task() {
    Create(2000, &parent_task);
    Exit();
}
