#include <ts7200.h>
#include <kernel.h>
#include <kern/io.h>
#include <kern/tasks.h>
#include <utils/timer.h>

extern void root_task();

void initialize() {
    // Initialize necessary APIs and libraries
    io_init();
    task_init();
    timer_init();
    // Create first user task.
    task_create(-1, 16, &root_task);
}

void syscall_handle(int tid, int request) {
    Task *current_task = task_at(tid);
    if (request == SYSCALL_IO_GETC) {
        int server = (int) current_task->syscall_args[0]; // not used for now
        int uart   = (int) current_task->syscall_args[1];
        current_task->return_value = io_getc(uart);
    }
    if (request == SYSCALL_IO_PUTC) {
        int server = (int) current_task->syscall_args[0]; // not used for now
        int uart   = (int) current_task->syscall_args[1];
        int ch     = (int) current_task->syscall_args[2];
        current_task->return_value = io_putc(uart, ch);
    }
    if (request == SYSCALL_TASK_CREATE) {
        unsigned int priority = (unsigned int) current_task->syscall_args[0];
        void *entry = (void *) current_task->syscall_args[1];
        current_task->return_value = task_create(tid, priority, entry);
    }
    if (request == SYSCALL_TASK_CREATE || request == SYSCALL_TASK_YIELD) {
        current_task->status = Ready;
    }
    if (request == SYSCALL_TASK_EXIT) {
        task_kill(tid);
    }
    if (request == SYSCALL_TASK_GETTID) {
        current_task->return_value = tid;
    }
    if (request == SYSCALL_TASK_GETPTID) {
        current_task->return_value = current_task->ptid;
    }
}

void kernel_entry() {
    initialize();  // includes starting the first user task
    for (;;) {
        unsigned int nextTID = task_schedule();
        if (nextTID == -1) break;
        int request = task_activate(nextTID);
        syscall_handle(nextTID, request);
    }
}
