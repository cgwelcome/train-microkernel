#include <hardware/syscon.h>
#include <server/idle.h>
#include <server/io.h>
#include <user/io.h>
#include <user/name.h>
#include <user/tasks.h>
#include <utils/bwio.h>

static void idle_task() {
    int io_server_id = WhoIs(IO_SERVER_NAME);
    if (io_server_id < 0) {
        io_server_id = CreateIOServer(3000, 3000, 3000);
    }

    const char *save_cursor = "\033[s";
    const char *retn_cursor = "\033[u";
    const char *clear_line  = "\033[K";
    const char *idle_rate_title_position = "\033[1;1H";
    const char *idle_rate_value_position = "\033[1;16H";

    syscon_lock(OFF);
    syscon_config(SHENA_MASK);
    Printf(io_server_id, COM2, "%s%sCPU Idle Rate: %s", save_cursor, idle_rate_title_position, retn_cursor);
    Yield();
    for (unsigned int i = 0; ; i++) {
        syscon_halt();
        if (i % 10 == 0) {
            int usage = MyCpuUsage();
            Printf(io_server_id, COM2, "%s%s%s%u.%u%%%s", save_cursor, idle_rate_value_position, clear_line, usage / 100, usage % 100, retn_cursor);
        }
    }
    Exit();
}

int CreateIdleTask(uint32_t priority) {
    return Create(priority, &idle_task);
}
