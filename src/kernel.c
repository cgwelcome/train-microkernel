#include <ts7200.h>
#include <kernel.h>
#include <kern/tasks.h>

extern void task_main();

void initialize() {
    // TODO: turn off interrupts in the ICU

    // Initialize variables related to task APIs.
    task_init();
    // Create first user task.
    task_create(-1, 0, &task_main);
}

void syscall_handle(int tid, int request) {
    if (request == SYSCALL_TASK_CREATE) {
        unsigned int priority = (unsigned int) task_at(tid)->syscall_args[0];
        void *entry = (void *) task_at(tid)->syscall_args[1];
        task_at(tid)->return_value = task_create(tid, priority, entry);
    }
    if (request == SYSCALL_TASK_CREATE || request == SYSCALL_TASK_YIELD) {
        task_at(tid)->status = Ready;
    }
    if (request == SYSCALL_TASK_EXIT) {
        task_kill(tid);
    }
}

void kmain() {
    initialize();  // includes starting the first user task
    for (;;) {
        unsigned int nextTID = task_schedule();
        if (nextTID == -1) break;
        int request = task_activate(nextTID);
        syscall_handle(nextTID, request);
    }
}