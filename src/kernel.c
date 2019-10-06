#include <ts7200.h>
#include <kernel.h>
#include <event.h>
#include <application.h>
#include <kern/io.h>
#include <kern/tasks.h>
#include <kern/switchframe.h>
#include <kern/irq.h>
#include <server/ns.h>
#include <utils/bwio.h>
#include <utils/timer.h>
#include <utils/icu.h>

void initialize() {
    // Enable L1I/L1D cache
    asm volatile("mrc p15, 0, r0, c1, c0, 0" ::: "r0");
    asm volatile("orr r0, r0, #(0x1 << 12)"  ::: "r0");
    asm volatile("orr r0, r0, #(0x1 << 2)"   ::: "r0");
    asm volatile("mcr p15, 0, r0, c1, c0, 0");
    // Initialize necessary APIs and libraries
    io_init();
    task_init();
    irq_init();
    // Initialize software and hardware handlers
    swi_handler_init();
    hwi_handler_init();
    // Initialiaze timers for interrupts
    timer_init(TIMER2, TIMER_IRQ_INTERVAL, TIMER_LOWFREQ);
    timer_init(TIMER3, TIMER_MAXVAL, TIMER_HIGHFREQ);
    icu_init();
    icu_activate(TC2UI_EVENT);
    // Initialize global variables for servers
    InitNS();
    // Create first user task.
    task_create(-1, 500, &irqtest_root_task);
}

void syscall_handle(int tid, int request) {
    Task *current_task = task_at(tid);
    if (request == HW_INTERRUPT) {
        irq_handle();
    }
    else if (request == SYSCALL_IO_GETC) {
        int server = (int) current_task->syscall_args[0]; // not used for now
        int uart   = (int) current_task->syscall_args[1];
        current_task->tf->r0 = io_getc(uart);
    }
    else if (request == SYSCALL_IO_PUTC) {
        int server = (int) current_task->syscall_args[0]; // not used for now
        int uart   = (int) current_task->syscall_args[1];
        int ch     = (int) current_task->syscall_args[2];
        current_task->tf->r0 = io_putc(uart, ch);
    }
    else if (request == SYSCALL_TASK_CREATE) {
        unsigned int priority = (unsigned int) current_task->syscall_args[0];
        void *entry = (void *) current_task->syscall_args[1];
        current_task->tf->r0 = task_create(tid, priority, entry);
    }
    else if (request == SYSCALL_TASK_EXIT) {
        task_kill(tid);
        ipc_cleanup(tid);
    }
    else if (request == SYSCALL_TASK_GETTID) {
        current_task->tf->r0 = tid;
    }
    else if (request == SYSCALL_TASK_GETPTID) {
        current_task->tf->r0 = current_task->ptid;
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
    else if (request == SYSCALL_IRQ_AWAITEVENT) {
        int eventtype = (int) current_task->syscall_args[0];
        irq_await(tid, eventtype);
    }
    else if (request == SYSCALL_DIAG_CPUUSAGE) {
        current_task->tf->r0 = task_cpuusage(tid);
    }

}

void kernel_entry() {
    initialize();  // includes starting the first user task
    unsigned int start_time = timer_read(TIMER3);
    task_setstarttime(start_time);
    for (;;) {
        unsigned int nextTID = task_schedule();
        if (nextTID == -1) break;
        unsigned int request = task_activate(nextTID);
        syscall_handle(nextTID, request);
    }
    int end_time = timer_read(TIMER3);
    bwprintf(COM2, "Kernel terminates after %u ms.", end_time - start_time);
}
