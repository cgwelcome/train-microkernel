#include <ts7200.h>
#include <lib/bwio.h>
#include <lib/kassert.h>
#include <user/tasks.h>
#include <user/ipc.h>
#include <application.h>
#include <stddef.h>

void sender_task() {
    int msglen = 8;
    const char *msg = "Wei Wei";
    Send(MyParentTid(), msg, msglen, NULL, 0);
    Exit();
}

void receiver_task() {
    int tid;
    int msglen = 8;
    char msg[msglen];
    int sendtid = Create(2000, &sender_task);
    Receive(&tid, msg, msglen);
    kassert(tid == sendtid);
    kassert(tid == -sendtid);
    bwprintf(COM2, "%s == %s\n\r", "Wei Wei", msg);
    Exit();
}

void ipctest_root_task() {
    Create(1000, &receiver_task);
    Exit();
}
