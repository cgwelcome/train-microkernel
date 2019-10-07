#include <application.h>
#include <user/tasks.h>
#include <utils/bwio.h>

void child_task() {
    bwprintf(COM2, "TID: %d, PTID: %d\n\r", MyTid(), MyParentTid());
    Yield();
    bwprintf(COM2, "TID: %d, PTID: %d\n\r", MyTid(), MyParentTid());
    Exit();
}

void k1_root_task() {
    bwprintf(COM2, "Created: %d\n\r", Create(1, &child_task));
    bwprintf(COM2, "Created: %d\n\r", Create(1, &child_task));
    bwprintf(COM2, "Created: %d\n\r", Create(1000, &child_task));
    bwprintf(COM2, "Created: %d\n\r", Create(1000, &child_task));
    bwprintf(COM2, "FirstUserTask: exiting\n\r");
    Exit();
}
