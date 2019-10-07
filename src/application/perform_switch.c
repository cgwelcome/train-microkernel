#include <application.h>
#include <user/tasks.h>

#define SWITCH_NUM 10000
#define WORKER_NUM 10

static void switch_perform_worker_task() {
    for (int i = 0; i < SWITCH_NUM; i++) Yield();
    Exit();
}

void switch_perform_root_task() {
    for (int i = 0; i < WORKER_NUM; i++) Create(100, &switch_perform_worker_task);
    Exit();
}
