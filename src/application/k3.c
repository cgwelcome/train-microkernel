#include <stddef.h>
#include <server/clock.h>
#include <server/idle.h>
#include <server/name.h>
#include <user/clock.h>
#include <user/ipc.h>
#include <user/name.h>
#include <user/tasks.h>
#include <utils/bwio.h>

typedef struct {
    int ticks;
    int num;
} DelayRequest;

static void child_task() {
    int mytid = MyTid();
    DelayRequest request;
    Send(MyParentTid(), NULL, 0, (char *)&request, sizeof(request));
    int cstid = WhoIs("CS");
    for (int i = 0; i < request.num; i++) {
        Delay(cstid, request.ticks);
        bwprintf(COM2, "TID: %d - Ticks: %d - Num: %d\n\r", mytid, request.ticks, i+1);
    }
    Exit();
}

static void delay_reply(int ticks, int num) {
    int tid;
    DelayRequest request;
    Receive(&tid, NULL, 0);
    request.ticks = ticks;
    request.num = num;
    Reply(tid, (char *)&request, sizeof(request));
}

void k3_root_task() {
    CreateNameServer(4000);
    CreateClockServer(3500);
    CreateIdleTask(1);

    Create(600, &child_task);
    Create(500, &child_task);
    Create(400, &child_task);
    Create(300, &child_task);

    delay_reply(10, 20);
    delay_reply(23, 9);
    delay_reply(33, 6);
    delay_reply(71, 3);

    Exit();
}
