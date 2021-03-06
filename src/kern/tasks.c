#include <arm.h>
#include <kernel.h>
#include <hardware/timer.h>
#include <kern/tasks.h>
#include <utils/assert.h>
#include <utils/queue.h>

static uint32_t total_task_count;
static uint32_t alive_task_count;
static uint32_t total_task_priority;
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
    assert(tid >= 0 && tid < MAX_TASK_NUM);
    return (tasks + tid);
}

int task_create(int ptid, uint32_t priority, void (*entry)(), uint32_t arg) {
    if (priority == 0 || priority > MAX_TASK_PRIORITY) {
        throw("invalid priority %u", priority);
    }
    if (total_task_count == MAX_TASK_NUM) {
        throw("out of task descriptors");
    }

    // Initialize task descriptor
    int tid = (int) total_task_count;
    Task new_task = {
        .status = READY,
        .tid = tid,
        .ptid = ptid,
        .runtime = 0,
        .priority = priority,
        .pc = (uint32_t) entry,
        .spsr = PSR_MODE_USR | PSR_FINT_DISABLED,
    };
    queue_init(&new_task.send_queue);
    // Initialize task stack
    asm("msr cpsr, %0" : : "I" (PSR_INT_DISABLED | PSR_FINT_DISABLED | PSR_MODE_SYS)); // enter system mode
        asm("mov sp, %0" : : "r" (ADDR_KERNEL_STACK_TOP - (uint32_t) tid * TASK_STACK_SIZE));
        asm("mov lr, #0x00"); // assume all the tasks will call Exit at the end.
        asm("mov r0, %0" : : "r" (arg));
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
    uint64_t min_vtime = UINT64_MAX;
    for (int tid = 0; tid < (int) total_task_count; tid++) {
        if (tasks[tid].status != READY) continue;
        uint64_t time = tasks[tid].runtime;
        uint64_t priority = tasks[tid].priority;
        uint64_t vtime = (time * total_task_priority) / priority;
        if (vtime < min_vtime) {
            min_vtime = vtime;
            ret_tid = tid;
        }
    }
    return ret_tid;
}

uint32_t task_activate(int tid) {
    Task *task = task_at(tid);

    task->status = ACTIVE;
    uint64_t task_start = timer_read_raw(TIMER3);
    uint32_t swi_code   = switch_frame(&task->pc, &task->tf, &task->spsr);
    uint64_t task_end   = timer_read_raw(TIMER3);
    task->status = READY;
    task->runtime += task_end - task_start;

    swi_code = swi_code & 0xFFFFFF;
    return swi_code;
}

void task_kill(int tid) {
    Task *task = task_at(tid);

    if (task->status == UNUSED) return;
    if (task->status != ZOMBIE) {
        task->status = ZOMBIE;
        alive_task_count -= 1;
        total_task_priority -= task->priority;
    }
}

void task_shutdown() {
    for (int tid = 0; tid < MAX_TASK_NUM; tid++) {
        if (tasks[tid].status == UNUSED) break;
        task_kill(tid);
    }
}
