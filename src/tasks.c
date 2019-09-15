#include <float.h>

#include <ts7200.h>
#include <kernel.h>
#include <tasks.h>

extern TaskGroup k_tasks;

int Create(unsigned int priority, void (*function)()) {
    if (priority > MAX_TASK_PRIORITY) {
        return -1; // invalid priority
    }

    unsigned int available_tid = MAX_TASK_NUM;
    for (unsigned int tid = 0; tid < MAX_TASK_NUM; tid++) {
        if (k_tasks.tasks[tid].status == Unused) {
            available_tid = tid;
            break;
        }
        // TODO: reuse Zombie tasks in the future
    }
    if (available_tid == MAX_TASK_NUM) {
        return -2; // out of task descriptors.
    }

    Task new_task = {
        .status = Ready,
        .tid = available_tid,
        .ptid = MyParentTid(),
        .runtime = 0,
        .priority = priority,
        .code = function,
        .stack = (void *) (ADDR_KERNEL_STACK_TOP - available_tid * TASK_STACK_SIZE),
    };
    k_tasks.alive_count += 1;
    k_tasks.total_priority += priority;
    k_tasks.tasks[available_tid] = new_task;
    return available_tid;
}

unsigned int Schedule() {
    unsigned int ret_tid;
    double min_vtime = DBL_MAX;
    for (unsigned int tid = 0; tid < MAX_TASK_NUM; tid++) {
        if (k_tasks.tasks[tid].status == Unused) continue;
        if (k_tasks.tasks[tid].status == Zombie) continue;
        unsigned int time = k_tasks.tasks[tid].runtime;
        unsigned int priority = k_tasks.tasks[tid].priority;
        double vtime = (double) (time * k_tasks.total_priority) / priority;
        if (vtime < min_vtime) {
            min_vtime = vtime;
            ret_tid = tid;
        }
    }
    return ret_tid;
}

int Activate(unsigned int tid) {
    return 0;
}

unsigned int MyTid() {
    return 0; // TODO: implement
}

unsigned int MyParentTid() {
    return 0; // TODO: implement
}

void Yield();

void Exit() {
    // TODO: k_tasks.alive_count -= 1;
}
