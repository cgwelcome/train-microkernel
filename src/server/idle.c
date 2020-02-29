#include <kernel.h>
#include <hardware/syscon.h>
#include <server/idle.h>
#include <user/io.h>
#include <user/name.h>
#include <user/tasks.h>
#include <user/ui.h>

static void idle_task() {
    int io_tid = WhoIs(SERVER_NAME_IO);

    syscon_lock(OFF);
    syscon_config(SHENA_MASK);
    Yield();
    for (unsigned int i = 0; ; i++) {
        syscon_halt();
        if (i % 10 == 0) {
            int usage = MyCpuUsage();
            PrintIdle(io_tid, usage);
        }
    }
    Exit();
}

void CreateIdleTask() {
    Create(PRIORITY_IDLE_TASK, &idle_task);
}
