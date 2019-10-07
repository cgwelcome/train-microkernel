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

unsigned long boot_time, halt_time;

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
    timer_init(TIMER3, TIMER_MAXVAL, TIMER_HIGHFREQ);
    icu_init();
    // Initialize global variables for servers
    InitNS();
    // Create first user task.
    task_create(-1, 500, &k3_root_task);
}

void handle_request(int tid, int request) {
    Task *current_task = task_at(tid);
    if (request == HW_INTERRUPT) {
        irq_handle();
    }
    else if (request == SYSCALL_IO_GETC) {
        int server = (int) current_task->tf->r0; // not used for now
        int uart   = (int) current_task->tf->r1;
        current_task->tf->r0 = io_getc(uart);
    }
    else if (request == SYSCALL_IO_PUTC) {
        int server = (int) current_task->tf->r0; // not used for now
        int uart   = (int) current_task->tf->r1;
        int ch     = (int) current_task->tf->r2;
        current_task->tf->r0 = io_putc(uart, ch);
    }
    else if (request == SYSCALL_TASK_CREATE) {
        unsigned int priority = (unsigned int) current_task->tf->r0;
        void *entry           = (void *)       current_task->tf->r1;
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
        int recvtid = (int)    current_task->tf->r0;
        char *msg   = (char *) current_task->tf->r1;
        int msglen  = (int)    current_task->tf->r2;
        char *reply = (char *) current_task->tf->r3;
        int rplen   = (int)    current_task->tf->r4;
        ipc_send(tid, recvtid, msg, msglen, reply, rplen);
    }
    else if (request == SYSCALL_IPC_RECV) {
        int *sendtid = (int *)  current_task->tf->r0;
        char *msg    = (char *) current_task->tf->r1;
        int msglen   = (int)    current_task->tf->r2;
        ipc_receive(tid, sendtid, msg, msglen);
    }
    else if (request == SYSCALL_IPC_REPLY) {
        int replytid = (int)    current_task->tf->r0;
        char *reply  = (char *) current_task->tf->r1;
        int rplen    = (int)    current_task->tf->r2;
        ipc_reply(tid, replytid, reply, rplen);
    }
    else if (request == SYSCALL_IRQ_AWAITEVENT) {
        int eventtype = (int) current_task->tf->r0;
        irq_await(tid, eventtype);
    }
    else if (request == SYSCALL_DIAG_CPUUSAGE) {
        unsigned long total_runtime = timer_read_raw(TIMER3) - boot_time;
        current_task->tf->r0 = current_task->runtime * 100 / total_runtime;
    }
}

void kernel_entry() {
    initialize();  // includes starting the first user task
    boot_time = timer_read_raw(TIMER3);
    for (;;) {
        unsigned int nextTID = task_schedule();
        if (nextTID == -1) break;
        unsigned int request = task_activate(nextTID);
        handle_request(nextTID, request);
    }
    halt_time = timer_read_raw(TIMER3);
    bwprintf(COM2, "Kernel terminates after %u ms.", (halt_time - boot_time)/TIMER_HIGHFREQ);
}
