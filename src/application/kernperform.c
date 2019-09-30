#include <ts7200.h>
#include <utils/bwio.h>
#include <utils/kassert.h>
#include <user/tasks.h>
#include <user/ipc.h>
#include <application.h>
#include <stddef.h>
#include <string.h>

#define MESSAGESIZE 256
#define RECEIVERFIRST 0
#define SRR_NUM 100000

static void recvmsg() {
    char msg[MESSAGESIZE];
    kassert(sizeof(msg) == MESSAGESIZE);
    int tid;
    for (int i = 0; i < SRR_NUM; i++) {
        Receive(&tid, msg, sizeof(MESSAGESIZE));
        Reply(tid, msg, sizeof(MESSAGESIZE));
    }
}

static void sendreplymsg(int tid) {
    char sendmsg[MESSAGESIZE];
    char replymsg[MESSAGESIZE];
    kassert(sizeof(sendmsg) == MESSAGESIZE);
    kassert(sizeof(replymsg) == MESSAGESIZE);
    for (int i = 0; i < SRR_NUM; i++) {
        Send(tid, sendmsg, sizeof(MESSAGESIZE), replymsg, sizeof(MESSAGESIZE));
    }
}

static void child_task() {
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
