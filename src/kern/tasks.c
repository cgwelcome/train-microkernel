#include <kern/switchframe.h>
#include <kern/tasks.h>
#include <arm.h>
#include <limits.h>
#include <stddef.h>
#include <utils/queue.h>
#include <utils/timer.h>
#include <utils/bwio.h>
#include <utils/kassert.h>
#include <utils/bwio.h>

static unsigned int total_task_count;
static unsigned int alive_task_count;
static unsigned int total_task_priority;
static Task tasks[MAX_TASK_NUM];

void task_init() {
    total_task_count = 0;
    alive_task_count = 0;
    total_task_priority = 0;
    for (int tid = 0; tid < MAX_TASK_NUM; tid++) {
        tasks[tid].status = UNUSED;
    }
}

Task *task_at(int tid) {
    if (tid < 0 || tid >= MAX_TASK_NUM) {
        return NULL;
    }
    return (tasks + tid);
}

int task_create(int ptid, unsigned int priority, void (*entry)()) {
    if (priority == 0 || priority > MAX_TASK_PRIORITY) {
        return -1; // invalid priority
    }
    if (total_task_count == MAX_TASK_NUM) {
        return -2; // out of task descriptors.
    }

    // Initialize task descriptor
    unsigned int tid = total_task_count;
    Task new_task = {
        .status = READY,
        .tid = tid,
        .ptid = ptid,
        .runtime = SCHEDULER_CALIBRATION,
        .priority = priority,
        .pc = (unsigned int) entry,
        .tf = (Trapframe *) (ADDR_KERNEL_STACK_TOP - (unsigned int) tid * TASK_STACK_SIZE),
        .spsr = PSR_MODE_USR | PSR_INT_DISABLED, // enter system mode
        .return_value = 0,
    };
    queue_init(&new_task.send_queue);
    // Initialize task stack
    asm("msr cpsr, %0" : : "I" (PSR_INT_DISABLED | PSR_FINT_DISABLED | PSR_MODE_SYS)); // enter system mode
        asm("ldr sp, %0" : : "m" (new_task.tf));
        asm("mov lr, #0x00"); // assume all the tasks will call Exit at the end.
        asm("push {r0-r12, lr}");
        asm("str sp, %0" : : "m" (new_task.tf));
    asm("msr cpsr, %0" : : "I" (PSR_INT_DISABLED | PSR_FINT_DISABLED | PSR_MODE_SVC)); // back to supervisor mode
    // Update internal storage
    total_task_count += 1;
    alive_task_count += 1;
    total_task_priority += priority;
    tasks[tid] = new_task;
    return tid;
}

int task_schedule() {
    if (alive_task_count == 0) return -1;

    int ret_tid = -1;
    unsigned int min_vtime = UINT_MAX;
    for (int tid = 0; tid < total_task_count; tid++) {
        if (tasks[tid].status != READY) continue;
        unsigned int time = tasks[tid].runtime;
        unsigned int priority = tasks[tid].priority;
        unsigned int vtime = (time * total_task_priority) / priority;
        if (vtime < min_vtime) {
            min_vtime = vtime;
            ret_tid = tid;
        }
    }
    return ret_tid;
}

int task_activate(int tid) {
    Task *current_task = task_at(tid);
    int swi_code, swi_argc, *swi_argv;

    current_task->status = ACTIVE;
    unsigned int task_start = timer_read_raw();
    swi_code = switchframe(&current_task->pc, &current_task->tf, &current_task->spsr);
    current_task->status = READY;
    current_task->runtime += timer_read_raw() - task_start;

    swi_argc = current_task->tf->r1;
    swi_argv = current_task->tf->r2;
    if (swi_argc > MAX_SYSCALL_ARG_NUM) swi_argc = MAX_SYSCALL_ARG_NUM;
    for (unsigned int i = 0; i < swi_argc; i++) {
        current_task->syscall_args[i] = swi_argv[i];
    }
    swi_code = swi_code & 0xFFFFFF;
    return swi_code;
}

void task_kill(int tid) {
    tasks[tid].status = ZOMBIE;
    alive_task_count -= 1;
    total_task_priority -= tasks[tid].priority;
}
