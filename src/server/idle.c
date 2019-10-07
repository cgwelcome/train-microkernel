#include <hardware/syscon.h>
#include <server/idle.h>
#include <user/tasks.h>
#include <utils/bwio.h>

static void idle_task() {
    syscon_lock(OFF);
    syscon_config(SHENA_MASK);
    bwputc(COM2, (char)27);
    bwprintf(COM2, "[s");
    // Jump cursor
    bwputc(COM2, (char)27);
    bwprintf(COM2, "[1;1H");
    // Print
    bwprintf(COM2, "CPU Usage: ");
    // Restore
    bwputc(COM2, (char)27);
    bwprintf(COM2, "[u");
    for (;;) {
        syscon_halt();
        // Save cursor
        bwputc(COM2, (char)27);
        bwprintf(COM2, "[s");
        // Jump cursor
        bwputc(COM2, (char)27);
        bwprintf(COM2, "[1;12H");
        // Clear line
        bwputc(COM2, (char)27);
        bwprintf(COM2, "[K");
        // Print
        bwprintf(COM2, "%d%%", MyCpuUsage());
        // Restore
        bwputc(COM2, (char)27);
        bwprintf(COM2, "[u");
    }
}

int CreateIdleTask(unsigned int priority) {
    return Create(priority, &idle_task);
}
