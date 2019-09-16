#include <ts7200.h>
#include <kernel.h>
#include <kern/tasks.h>

void task_main() {
    return;
}

void initialize() {
    // TODO: turn off interrupts in the ICU

    // Initialize variables related to task APIs.
    task_init();
    // Create first user task.
    task_create(-1, 0, &task_main);
}

void syscall_handle(int request) {
    return;
}

void kmain() {
    initialize();  // includes starting the first user task
    for (;;) {
        unsigned int nextTID = task_schedule();
        if (nextTID == -1) break;
        int request = task_activate(nextTID);
        syscall_handle(request);
    }
}
