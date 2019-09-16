#include <kern/tasks.h>
#include <float.h>

unsigned int alive_task_count;
unsigned int total_priority;
Task tasks[MAX_TASK_NUM];

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
        .entry = entry,
        .stack = (void *) (ADDR_KERNEL_STACK_TOP - available_tid * TASK_STACK_SIZE),
        .return_value = 0,
    };
    alive_task_count += 1;
    total_priority += priority;
    tasks[available_tid] = new_task;
    return available_tid;
}

int task_schedule() {
    if (alive_task_count == 0) return -1;

    int ret_tid;
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

void task_zygote(void (*entry)()) {
    void *swi_handler = (void *)0x28;
    asm("str lr, %0" : : "m" (swi_handler));
    // TODO: drop supervisor mode
    asm("mov r1, %0" : : "r" (entry));
    asm("bx r1");
}


void task_kill(int tid) {
    tasks[tid].status = Zombie;
    alive_task_count -= 1;
    total_priority -= tasks[tid].priority;
}
