#include <ts7200.h>
#include <bwio.h>
#include <user/tasks.h>

void child_task() {
    bwprintf(COM2, "TID: %d, PTID: %d\n", MyTid(), MyParentTid());
    Yield();
    bwprintf(COM2, "TID: %d, PTID: %d\n", MyTid(), MyParentTid());
    Exit();
}

void root_task() {
    bwprintf(COM2, "Created: %d\n", Create(5, &child_task));
    bwprintf(COM2, "Created: %d\n", Create(5, &child_task));
    bwprintf(COM2, "Created: %d\n", Create(15, &child_task));
    bwprintf(COM2, "Created: %d\n", Create(15, &child_task));
    bwprintf(COM2, "FirstUserTask: exiting\n");
    Exit();
}
