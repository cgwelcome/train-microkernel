#include <ts7200.h>
#include <user/tasks.h>
#include <utils/bwio.h>

void child_task() {
    bwprintf(COM2, "TID: %d, PTID: %d\n", MyTid(), MyParentTid());
    Yield();
    bwprintf(COM2, "TID: %d, PTID: %d\n", MyTid(), MyParentTid());
    Exit();
}

void root_task() {
    bwprintf(COM2, "Created: %d\n", Create(8, &child_task));
    bwprintf(COM2, "Created: %d\n", Create(8, &child_task));
    bwprintf(COM2, "Created: %d\n", Create(32, &child_task));
    bwprintf(COM2, "Created: %d\n", Create(32, &child_task));
    bwprintf(COM2, "FirstUserTask: exiting\n");
    Exit();
}
