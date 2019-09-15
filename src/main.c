#include <ts7200.h>
#include <kernel.h>
#include <tasks.h>

TaskGroup k_tasks;

void empty_task() {
    return;
}

void initialize() {
    // Initialize k_tasks
    k_tasks.alive_count = 0;
    k_tasks.total_priority = 0;
    for (unsigned int tid = 0; tid < MAX_TASK_NUM; tid++) {
        k_tasks.tasks[tid].status = Unused;
    }
    // Create first user task
    Create(0, &empty_task);
}

void kmain() {
    initialize();  // includes starting the first user task
    for (;;) {
        unsigned int nextTID = Schedule();
        int request = Activate(nextTID);
        switch (request) {
        default:
            break;
        }
    }
}
