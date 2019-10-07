#include <hardware/syscon.h>
#include <server/idle.h>
#include <user/tasks.h>
#include <utils/bwio.h>

static void idle_task() {
    syscon_lock(OFF);
    syscon_config(SHENA_MASK);
    for (;;) {
        /*bwprintf(COM2, "CPU Usage: %d%%\n\r", MyCpuUsage());*/
        syscon_halt();
    }
}

int CreateIdleTask(unsigned int priority) {
    return Create(priority, &idle_task);
}
