#include <ts7200.h>
#include <kernel.h>
#include <application.h>
#include <kern/io.h>
#include <kern/tasks.h>
#include <utils/bwio.h>
#include <utils/timer.h>

void initialize() {
    // Enable cache
    asm volatile("mrc p15, 0, r0, c1, c0, 0");
    asm volatile("orr r0, r0, #(0x1 << 12)");
	asm volatile("orr r0, r0, #(0x1 << 2)");
    asm volatile("mcr p15, 0, r0, c1, c0, 0");
    // Initialize necessary APIs and libraries
    io_init();
    task_init();
    timer_init();
    // Create first user task.
    task_create(-1, 500, &k2_root_task);
}

void syscall_handle(int tid, int request) {
    Task *current_task = task_at(tid);
    if (request == SYSCALL_IO_GETC) {
        int server = (int) current_task->syscall_args[0]; // not used for now
        int uart   = (int) current_task->syscall_args[1];
        current_task->return_value = io_getc(uart);
    }
    else if (request == SYSCALL_IO_PUTC) {
        int server = (int) current_task->syscall_args[0]; // not used for now
        int uart   = (int) current_task->syscall_args[1];
        int ch     = (int) current_task->syscall_args[2];
        current_task->return_value = io_putc(uart, ch);
    }
    else if (request == SYSCALL_TASK_CREATE) {
        unsigned int priority = (unsigned int) current_task->syscall_args[0];
        void *entry = (void *) current_task->syscall_args[1];
        current_task->return_value = task_create(tid, priority, entry);
    }
    else if (request == SYSCALL_TASK_EXIT) {
        task_kill(tid);
        ipc_cleanup(tid);
    }
    else if (request == SYSCALL_TASK_GETTID) {
        current_task->return_value = tid;
    }
    else if (request == SYSCALL_TASK_GETPTID) {
        current_task->return_value = current_task->ptid;
    }
    else if (request == SYSCALL_IPC_SEND) {
        int recvtid = (int) current_task->syscall_args[0];
        char *msg = (char *) current_task->syscall_args[1];
        int msglen = (int) current_task->syscall_args[2];
        char *reply = (char *) current_task->syscall_args[3];
        int rplen = (int) current_task->syscall_args[4];
        ipc_send(tid, recvtid, msg, msglen, reply, rplen);
    }
    else if (request == SYSCALL_IPC_RECV) {
        int *sendtid = (int *) current_task->syscall_args[0];
        char *msg = (char *) current_task->syscall_args[1];
        int msglen = (int) current_task->syscall_args[2];
        ipc_receive(tid, sendtid, msg, msglen);
    }
    else if (request == SYSCALL_IPC_REPLY) {
        int replytid = (int) current_task->syscall_args[0];
        char *reply = (char *) current_task->syscall_args[1];
        int rplen = (int) current_task->syscall_args[2];
        ipc_reply(tid, replytid, reply, rplen);
    }
}

void kernel_entry() {
    initialize();  // includes starting the first user task
    unsigned int start_time = timer_read();
    for (;;) {
        unsigned int nextTID = task_schedule();
        if (nextTID == -1) break;
        int request = task_activate(nextTID);
        syscall_handle(nextTID, request);
    }
    unsigned int end_time = timer_read();
    bwprintf(COM2, "Kernel terminates after %u ms.", end_time - start_time);
}
