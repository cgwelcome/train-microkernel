#include <kernel.h>
#include <server/clock.h>
#include <server/idle.h>
#include <server/name.h>
#include <user/clock.h>
#include <user/ipc.h>
#include <user/name.h>
#include <user/tasks.h>
#include <utils/bwio.h>

static void child_task() {
    int cstid = WhoIs(SERVER_NAME_CLOCK);
    bwprintf(COM2, "before");
    Delay(cstid, 500);
    bwprintf(COM2, "after");
    Exit();
}

void clock_test_root_task() {
    CreateClockServer();
    Create(2000, &child_task);
    CreateIdleTask();
    Exit();
}
