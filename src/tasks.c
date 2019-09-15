#include <float.h>

#include <ts7200.h>
#include <tasks.h>

extern TaskGroup k_tasks;

int Schedule() {
    unsigned int ret_tid;

    double min_vtime = DBL_MAX;
    unsigned int tid = 0;
    for (tid = 0; tid < MAX_TASK_NUM; tid++) {
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

int Activate(int TID) {
    return 0;
}
