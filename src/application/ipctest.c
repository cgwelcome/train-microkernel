#include <ts7200.h>
#include <lib/bwio.h>
#include <lib/kassert.h>
#include <user/tasks.h>
#include <user/ipc.h>
#include <application.h>
#include <stddef.h>
#include <string.h>

// Send String to the Parent Task
void recvblock_child_test() {
    const char *msg = "Wei-Wei";
    Send(MyParentTid(), msg, strlen(msg)+1, NULL, 0);
    kassert(1 == 2); // Shouldn't get here, no one to reply
    Exit();
}

// Send -> receive
// Receive Wei-Wei from child task
void recvblock_test() {
    int tid;
    int msglen = 9;
    char msg[msglen];
    int sendtid = Create(2000, &recvblock_child_test);
    Receive(&tid, msg, msglen);
    kassert(tid == sendtid);
    kassert(strcmp("Wei-Wei", msg) == 0);
    Exit();
}

// Error Code -1 test on Send
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

// Send a message, the receiver dies before it retrieve the message
void sendtransacterror_child_test() {
    int returncode = Send(MyParentTid(), NULL, 0, NULL, 0);
    kassert(returncode == -2);
    Exit();
}

// Error Code -2 test on Send
void sendtransacterror_test() {
    Create(2000, &sendtransacterror_child_test);
    Exit();
}

// Receive string from parent
void sendblock_child_test() {
    int tid;
    int msglen = 9;
    char msg[msglen];
    Receive(&tid, msg, msglen);
    kassert(strcmp("Wei-Wei", msg) == 0);
    kassert(tid == MyParentTid());
    Exit();
}

// Receive -> send
// The parent task is the receiver
void sendblock_test() {
    int childtid = Create(2000, &sendblock_child_test);
    const char *msg = "Wei-Wei";
    Send(childtid, msg, strlen(msg)+1, NULL, 0);
    Exit();
}

void reply_child_test() {
    int msglen = 9;
    char msg[msglen];
    Send(MyParentTid(), NULL, 0, msg, msglen);
    kassert(strcmp("Wei-Wei", msg) == 0);
    Exit();
}

// Send -> Receive -> Reply
void reply_test() {
    int tid;
    const char *msg = "Wei-Wei";

    Create(2000, &reply_child_test);
    Receive(&tid, NULL, 0);
    Reply(tid, msg, strlen(msg)+1);
    Exit();
}

void replyerror_test() {
    int returncode;
    returncode = Reply(-1, NULL, 0);
    kassert(returncode == -1);
    returncode = Reply(100, NULL, 0);
    kassert(returncode == -1);
    returncode = Reply(MyTid(), NULL, 0);
    kassert(returncode == -2);
    Exit();
}

void ipclength_child_test() {
    const char *sendmsg = "Wei-Wei";
    int replylen = 5;
    char replymsg[replylen];

    int returncode = Send(MyParentTid(), sendmsg, strlen(sendmsg)+1, replymsg, replylen);
    kassert(returncode == 5);
    kassert(strcmp(strcat(replymsg, "\0"), "NiHao") == 0);
    Exit();
}

void ipclength_test() {
    int tid;
    int receivelen = 3;
    char receivemsg[receivelen];
    int returncode;

    Create(2000, &ipclength_child_test);
    returncode = Receive(&tid, receivemsg, receivelen);
    kassert(returncode == receivelen);
    kassert(strcmp(strcat(receivemsg, "\0"), "Wei") == 0);

    const char *replymsg = "NiHaoMa";
    returncode = Reply(tid, replymsg, strlen(replymsg)+1);
    kassert(returncode == 5);
    Exit();
}

void ipctest_root_task() {
    Create(1000, &recvblock_test);
    Create(1000, &senderror_test);
    Create(1000, &sendtransacterror_test);
    Create(1000, &sendblock_test);
    Create(1000, &reply_test);
    Create(1000, &replyerror_test);
    Create(1000, &ipclength_test);
    Exit();
}
