#include <hardware/syscon.h>
#include <server/idle.h>
#include <user/tasks.h>
#include <utils/bwio.h>

static void idle_task() {
    syscon_lock(OFF);
    syscon_config(SHENA_MASK);
    // Print Title
    bwprintf(COM2, "\033[s");
        bwprintf(COM2, "\033[1;1H");
        bwprintf(COM2, "CPU Idle Rate: ");
    bwprintf(COM2, "\033[u");
    for (;;) {
        syscon_halt();
        // Print CPU usage
        bwprintf(COM2, "\033[s");
            bwprintf(COM2, "\033[1;16H");
            bwprintf(COM2, "\033[K");
            bwprintf(COM2, "%d%%", MyCpuUsage());
        bwprintf(COM2, "\033[u");
    }
}

int CreateIdleTask(uint32_t priority) {
    return Create(priority, &idle_task);
}
