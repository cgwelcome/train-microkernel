#include <ts7200.h>
#include <lib/bwio.h>
#include <lib/kassert.h>
#include <user/tasks.h>
#include <user/ipc.h>
#include <application.h>
#include <stddef.h>

void sendrecv_child_test() {
    int msglen = 8;
    const char *msg = "Wei Wei";
    Send(MyParentTid(), msg, msglen, NULL, 0);
    kassert(1 == 2); // Shouldn't get there, no one will Reply, kernel will clean up this task
    Exit();
}

// Send first, then receive test
void sendrecv_test() {
    int tid;
    int msglen = 8;
    char msg[msglen];
    int sendtid = Create(2000, &sendrecv_child_test);
    int returncode = Receive(&tid, msg, msglen);
    kassert(tid == sendtid);
    kassert(returncode == 0);
    bwprintf(COM2, "%s == %s\n\r", "Wei Wei", msg);
    Exit();
}


void senderror_test() {
    int returncode;
    /* Send to out of bound task*/
    returncode = Send(-1, NULL, 0, NULL, 0);
    kassert(returncode == -1);
    /* Send to an unused task*/
    Send(100, NULL, 0, NULL, 0);
    kassert(returncode == -1);
    Exit();
}


void sendtransacerror_child_test() {
    int returncode = Send(MyParentTid(), NULL, 0, NULL, 0);
    kassert(returncode == -2);
    Exit();
}

void sendtransacerror_test() {
    Create(2000, &sendtransacerror_child_test);
    Exit();
}

void ipctest_root_task() {
    Create(1000, &sendrecv_test);
    Create(1000, &senderror_test);
    Create(1000, &sendtransacerror_test);
    Exit();
}
