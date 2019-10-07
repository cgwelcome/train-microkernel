#include <arm.h>
#include <kernel.h>
#include <limits.h>
#include <stddef.h>
#include <hardware/timer.h>
#include <kern/tasks.h>
#include <utils/queue.h>

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
        .runtime = 0,
        .priority = priority,
        .pc = (unsigned int) entry,
        .spsr = PSR_MODE_USR | PSR_FINT_DISABLED,
    };
    queue_init(&new_task.send_queue);
    // Initialize task stack
    asm("msr cpsr, %0" : : "I" (PSR_INT_DISABLED | PSR_FINT_DISABLED | PSR_MODE_SYS)); // enter system mode
        asm("mov sp, %0" : : "r" (ADDR_KERNEL_STACK_TOP - (unsigned int) tid * TASK_STACK_SIZE));
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
        unsigned int vtime = ((time * total_task_priority) + SCHEDULER_CALIBRATION)/priority;
        if (vtime < min_vtime) {
            min_vtime = vtime;
            ret_tid = tid;
        }
    }
    return ret_tid;
}

int task_activate(int tid) {
    Task *current_task = task_at(tid);

    current_task->status = ACTIVE;
    uint64_t task_start = timer_read_raw(TIMER3);
    int swi_code = switch_frame(&current_task->pc, &current_task->tf, &current_task->spsr);
    uint64_t task_end   = timer_read_raw(TIMER3);
    current_task->status = READY;
    current_task->runtime += task_end - task_start;

    swi_code = swi_code & 0xFFFFFF;
    return swi_code;
}

void task_kill(int tid) {
    tasks[tid].status = ZOMBIE;
    alive_task_count -= 1;
    total_task_priority -= tasks[tid].priority;
}
