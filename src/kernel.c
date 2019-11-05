#include <kernel.h>
#include <application.h>
#include <priority.h>
#include <stdint.h>
#include <hardware/icu.h>
#include <hardware/timer.h>
#include <hardware/uart.h>
#include <kern/event.h>
#include <kern/tasks.h>
#include <server/clock.h>
#include <server/io.h>
#include <server/name.h>
#include <utils/assert.h>
#include <utils/bwio.h>

uint64_t boot_time, halt_time;

void initialize() {
    // Enable L1I/L1D cache
    __asm__ volatile("mrc p15, 0, r0, c1, c0, 0" ::: "r0");
    __asm__ volatile("orr r0, r0, #(0x1 << 12)"  ::: "r0");
    __asm__ volatile("orr r0, r0, #(0x1 << 2)"   ::: "r0");
    __asm__ volatile("mcr p15, 0, r0, c1, c0, 0");
    // Initialiaze necessary hardwares
    icu_init();
    timer_init(TIMER3, TIMER_MAXVAL, TIMER_HIGHFREQ);
    // Initialize necessary APIs and libraries
    task_init();
    event_init();
    // Initialize software and hardware handlers
    swi_handler_init();
    hwi_handler_init();
    // Initialize global variables for servers
    InitIOServer();
    InitNameServer();
    InitClockServer();
    // Create first user task.
    task_create(-1, PRIORITY_ROOT_TASK, &k4_root_task);
}

void handle_request(int tid, uint32_t request) {
    Task *current_task = task_at(tid);
    if (request == HW_INTERRUPT) {
        event_handle();
    }
    else if (request == SYSCALL_TASK_CREATE) {
        uint32_t priority = (uint32_t) current_task->tf->r0;
        void *   entry    = (void *)   current_task->tf->r1;
        current_task->tf->r0 = (uint32_t) task_create(tid, priority, entry);
    }
    else if (request == SYSCALL_TASK_EXIT) {
        task_kill(tid);
        ipc_cleanup(tid);
    }
    else if (request == SYSCALL_TASK_GETTID) {
        current_task->tf->r0 = (uint32_t) tid;
    }
    else if (request == SYSCALL_TASK_GETPTID) {
        current_task->tf->r0 = (uint32_t) current_task->ptid;
    }
    else if (request == SYSCALL_TASK_CPUUSAGE) {
        uint64_t total_runtime = timer_read_raw(TIMER3) - boot_time;
        current_task->tf->r0 = (uint32_t) (current_task->runtime * 10000UL / total_runtime);
    }
    else if (request == SYSCALL_TASK_SHUTDOWN) {
        task_shutdown();
    }
    else if (request == SYSCALL_IPC_SEND) {
        int    recvtid = (int)    current_task->tf->r0;
        char * msg     = (char *) current_task->tf->r1;
        size_t msglen  = (size_t) current_task->tf->r2;
        char * reply   = (char *) current_task->tf->r3;
        size_t rplen   = (size_t) current_task->tf->r4;
        ipc_send(tid, recvtid, msg, msglen, reply, rplen);
    }
    else if (request == SYSCALL_IPC_RECV) {
        int *  sendtid = (int *)  current_task->tf->r0;
        char * msg     = (char *) current_task->tf->r1;
        size_t msglen  = (size_t) current_task->tf->r2;
        ipc_receive(tid, sendtid, msg, msglen);
    }
    else if (request == SYSCALL_IPC_PEEK) {
        int    peektid = (int)    current_task->tf->r0;
        char * msg     = (char *) current_task->tf->r1;
        size_t msglen  = (size_t) current_task->tf->r2;
        ipc_peek(tid, peektid, msg, msglen);
    }
    else if (request == SYSCALL_IPC_REPLY) {
        int    replytid = (int)    current_task->tf->r0;
        char * reply    = (char *) current_task->tf->r1;
        size_t rplen    = (size_t) current_task->tf->r2;
        ipc_reply(tid, replytid, reply, rplen);
    }
    else if (request == SYSCALL_IRQ_AWAITEVENT) {
        int event = (int) current_task->tf->r0;
        event_await(tid, event);
    }
    else if (request == SYSCALL_PANIC) {
        char * expr = (char *) current_task->tf->r0;
        char * file = (char *) current_task->tf->r1;
        int    line = (int)    current_task->tf->r2;
        panic(expr, file, line);
    }
}

void kernel_entry() {
    initialize();  // includes starting the first user task
    boot_time = timer_read_raw(TIMER3);
    for (;;) {
        int32_t nextTID = task_schedule();
        if (nextTID == -1) break;
        uint32_t request = task_activate(nextTID);
        handle_request(nextTID, request);
    }
    halt_time = timer_read_raw(TIMER3);
    bwprintf(COM2, "\033[30;1HKernel terminates after %u ms.", (halt_time - boot_time)/TIMER_HIGHFREQ);

    icu_disableall();
    uart_disable_all_interrupts(COM1);
    uart_disable_all_interrupts(COM2);
}
