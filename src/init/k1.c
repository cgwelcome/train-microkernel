#include <user/tasks.h>
#include <utils/bwio.h>

void child_task() {
    bwprintf(COM2, "TID: %d, PTID: %d\r\n", MyTid(), MyParentTid());
    Yield();
    bwprintf(COM2, "TID: %d, PTID: %d\r\n", MyTid(), MyParentTid());
    Exit();
}

void k1_root_task() {
    bwprintf(COM2, "Created: %d\r\n", Create(1, &child_task));
    bwprintf(COM2, "Created: %d\r\n", Create(1, &child_task));
    bwprintf(COM2, "Created: %d\r\n", Create(1000, &child_task));
    bwprintf(COM2, "Created: %d\r\n", Create(1000, &child_task));
    bwprintf(COM2, "FirstUserTask: exiting\r\n");
    Exit();
}
