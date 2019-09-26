#include <kern/tasks.h>
#include <arm.h>
#include <float.h>
#include <stddef.h>
#include <lib/queue.h>
#include <lib/timer.h>

static unsigned int alive_task_count;
static unsigned int total_priority;
static Task tasks[MAX_TASK_NUM];

void task_init() {
    alive_task_count = 0;
    total_priority = 0;
    for (int tid = 0; tid < MAX_TASK_NUM; tid++) {
        tasks[tid].status = UNUSED;
    }
}

Task *task_at(int tid) {
    if (tid >= MAX_TASK_NUM) {
        return NULL;
    }
    return (tasks + tid);
}

int task_create(int ptid, unsigned int priority, void (*entry)()) {
    if (priority == 0 || priority > MAX_TASK_PRIORITY) {
        return -1; // invalid priority
    }

    int available_tid = -1;
    for (int tid = 0; tid < MAX_TASK_NUM; tid++) {
        if (tasks[tid].status == UNUSED) {
            available_tid = tid;
            break;
        }
    }
    if (available_tid == -1) {
        return -2; // out of task descriptors.
    }

    // Initialize task descriptor
    Task new_task = {
        .status = READY,
        .tid = available_tid,
        .ptid = ptid,
        .runtime = 0,
        .priority = priority,
        .pc = (unsigned int) entry,
        .sp = (unsigned int) (ADDR_KERNEL_STACK_TOP - (unsigned int) available_tid * TASK_STACK_SIZE),
        .spsr = PSR_MODE_USR,
        .return_value = 0,
    };
    q_init(&new_task.send_queue);
    // Initialize task stack
    asm("msr cpsr, %0" : : "I" (PSR_INT_DISABLED | PSR_FINT_DISABLED | PSR_MODE_SYS)); // enter system mode
        asm("ldr sp, %0" : : "m" (new_task.sp));
        asm("mov lr, #0x00"); // assume all the tasks will call Exit at the end.
        asm("push {r3-r12, lr}");
        asm("str sp, %0" : : "m" (new_task.sp));
    asm("msr cpsr, %0" : : "I" (PSR_INT_DISABLED | PSR_FINT_DISABLED | PSR_MODE_SVC)); // back to supervisor mode
    alive_task_count += 1;
    total_priority += priority;
    tasks[available_tid] = new_task;
    return available_tid;
}

int task_schedule() {
    if (alive_task_count == 0) return -1;

    int ret_tid = -1;
    double min_vtime = DBL_MAX;
    for (int tid = 0; tid < MAX_TASK_NUM; tid++) {
        if (tasks[tid].status != READY) continue;
        unsigned int time = tasks[tid].runtime;
        unsigned int priority = tasks[tid].priority;
        double vtime = (double) ((time * total_priority)+SCHEDULER_CALIBRATION)/ priority;
        if (vtime < min_vtime) {
            min_vtime = vtime;
            ret_tid = tid;
        }
    }
    return ret_tid;
}

int task_activate(int tid) {
    Task *current_task = task_at(tid);
    unsigned int swi_code, swi_argc, *swi_argv;

    current_task->status = ACTIVE;
    unsigned int task_start = timer_read_raw();
        static unsigned int kernel_frame;
        asm("str fp, %0" : : "m" (kernel_frame));
        asm("" ::: "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r12", "lr", "memory");
        // Restore user context
        asm("msr spsr, %0" : : "r" (current_task->spsr));
        asm("ldr lr, %0"   : : "m" (current_task->pc));
        asm("msr cpsr, %0" : : "I" (PSR_INT_DISABLED | PSR_FINT_DISABLED | PSR_MODE_SYS)); // enter system mode
            asm("ldr r0, %0" : : "m" (current_task->return_value) : "r0");
            asm("ldr sp, %0" : : "m" (current_task->sp));
            asm("pop {r3-r12, lr}");
        asm("msr cpsr, %0" : : "I" (PSR_INT_DISABLED | PSR_FINT_DISABLED | PSR_MODE_SVC)); // back to supervisor mode
        asm("mov r1, #0x28" ::: "r1"); asm("str pc, [r1]"); asm("nop"); // setup swi handler
        // Switch to user context
        asm("movs pc, lr" ::: "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r12", "lr");
        // Store user context
        asm("msr cpsr, %0" : : "I" (PSR_INT_DISABLED | PSR_FINT_DISABLED | PSR_MODE_SYS)); // enter system mode
            asm("push {r3-r12, lr}");
            asm("mov r12, sp" ::: "r12");
        asm("msr cpsr, %0" : : "I" (PSR_INT_DISABLED | PSR_FINT_DISABLED | PSR_MODE_SVC)); // back to supervisor mode
        // Handle arguments passed for syscall
        asm("ldr fp, %0" : : "m" (kernel_frame));
        asm("ldr r0, [lr, #-4]" ::: "r0");
        asm("str r0, %0" : : "m" (swi_code));
        asm("str r1, %0" : : "m" (swi_argc));
        asm("str r2, %0" : : "m" (swi_argv));
        // Store important registers for current_task
        asm("str lr, %0" : : "m" (current_task->pc));
        asm("str r12, %0" : : "m" (current_task->sp));
        asm("mrs %0, spsr" : "=r" (current_task->spsr));
    current_task->status = READY;
    current_task->runtime += timer_read_raw() - task_start;

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
    total_priority -= tasks[tid].priority;
}
