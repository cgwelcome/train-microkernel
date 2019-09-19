#include <kern/tasks.h>
#include <float.h>
#include <bwio.h>

unsigned int alive_task_count;
unsigned int total_priority;
Task tasks[MAX_TASK_NUM];

unsigned int kernel_frame;

// TODO: set them in task_activate()
int current_tid;
int current_ptid;

void task_init() {
    alive_task_count = 0;
    total_priority = 0;
    for (int tid = 0; tid < MAX_TASK_NUM; tid++) {
        tasks[tid].status = Unused;
    }

    current_tid = -1; current_ptid = -1;
}

Task *task_at(int tid) {
    return (tasks + tid);
}

int task_create(int ptid, unsigned int priority, void (*entry)()) {
    if (priority == 0 || priority > MAX_TASK_PRIORITY) {
        return -1; // invalid priority
    }

    int available_tid = -1;
    for (int tid = 0; tid < MAX_TASK_NUM; tid++) {
        if (tasks[tid].status == Unused) {
            available_tid = tid;
            break;
        }
    }
    if (available_tid == -1) {
        return -2; // out of task descriptors.
    }

    Task new_task = {
        .status = Ready,
        .tid = available_tid,
        .ptid = ptid,
        .runtime = 0,
        .priority = priority,
        .pc = (unsigned int) entry,
        .sp = (unsigned int) (ADDR_KERNEL_STACK_TOP - (unsigned int) available_tid * TASK_STACK_SIZE),
        .spsr = SPSR_USER_MODE | SPSR_FIQ_INTERRUPT | SPSR_IRQ_INTERRUPT,
        .return_value = 0,
    };
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
        if (tasks[tid].status == Unused) continue;
        if (tasks[tid].status == Zombie) continue;
        unsigned int time = tasks[tid].runtime;
        unsigned int priority = tasks[tid].priority;
        double vtime = (double) (time * total_priority) / priority;
        if (vtime < min_vtime) {
            min_vtime = vtime;
            ret_tid = tid;
        }
    }
    return ret_tid;
}

void task_zygote(void (*entry)(), unsigned int spsr) {
    unsigned int *swi_handler = (unsigned int *)0x28;

    asm("str lr, [%0]" : : "r" (swi_handler));
    asm("mov lr, %0" : : "r" (entry));
    asm("msr spsr, %0" : : "r" (spsr));
    asm("movs pc, lr");
}

int task_activate(int tid) {
    unsigned int swi_instruction;
    unsigned int swi_argc;
    unsigned int *swi_argv;

    asm("push {r0-r10}");
    asm("mov %0, fp" : "=r" (kernel_frame));
    task_zygote((void (*)()) tasks[tid].pc, tasks[tid].spsr);
    asm("mov fp, %0" : : "r" (kernel_frame));
    asm("ldr %0, [lr, #-4]": "=r" (swi_instruction));
    asm("mov %0, r1" : "=r" (swi_argc));
    asm("mov %0, r2" : "=r" (swi_argv));
    asm("pop {r0-r10}");

    for (unsigned int i = 0; i < swi_argc; i++) {
        tasks[tid].syscall_args[i] = swi_argv[i];
    }
    swi_instruction = swi_instruction & 0xFFFFFF;
    return swi_instruction;
}
// TODO: update pc and sp of the task in activate
// TODO: update current_tid and current_ptid in activate

void task_kill(int tid) {
    tasks[tid].status = Zombie;
    alive_task_count -= 1;
    total_priority -= tasks[tid].priority;
}
