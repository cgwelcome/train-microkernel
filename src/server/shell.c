#include <server/clock.h>
#include <server/io.h>
#include <server/shell.h>
#include <server/trainset.h>
#include <user/io.h>
#include <user/clock.h>
#include <user/name.h>
#include <user/tasks.h>
#include <user/trainset.h>
#include <utils/queue.h>

#define SENSOR_LIST_SIZE 7

static void shell_execute_command(Queue *cmd_buffer) {
    Exit();
}

static void shell_keyboard_task() {
    int iotid = WhoIs(IO_SERVER_NAME);
    int traintid = WhoIs(TRAINSET_SERVER_NAME);
    static Queue cmd_buffer;
    queue_init(&cmd_buffer);
    for (;;) {
        int c = Getc(iotid, COM2);
        if (c == '\r' && queue_size(&cmd_buffer) > 0)  {
            if (queue_peek(&cmd_buffer) == 'q') {
                Trainset_Done(traintid);
                ShutdownIOServer();
                Shutdown();
            }
            shell_execute_command(&cmd_buffer);
            queue_init(&cmd_buffer);
        }
        if (c == '\b' && queue_size(&cmd_buffer) > 0) {
            queue_pop(&cmd_buffer);
        }
        if (c < CMD_BUFFER_SIZE) {
            queue_push(&cmd_buffer, c);
        }
    }
    Exit();
}

static void shell_clock_init(ShellClock *shellclock) {
    shellclock->minute = 0;
    shellclock->second = 0;
    shellclock->decisecond = 0;
}

static void shell_clock_update(ShellClock *shellclock) {
    if (shellclock->decisecond < 9) {
        shellclock->decisecond++;
        return;
    }
    shellclock->decisecond = 0;
    if (shellclock->second < 59) {
        shellclock->second++;
        return;
    }
    shellclock->second = 0;
    shellclock->minute++;
}

static void shell_clock_display(int iotid, ShellClock *shellclock) {
    Printf(iotid, COM2, "\033[%d;%dH\033[K%u:%u:%u",
            LINE_TIME, 1,
            shellclock->minute,
            shellclock->second,
            shellclock->decisecond
    );
}

static void shell_clock_task() {
    ShellClock shellclock;
    shell_clock_init(&shellclock);
    int clocktid = WhoIs(CLOCK_SERVER_NAME);
    int iotid = WhoIs(IO_SERVER_NAME);
    for (;;) {
        Delay(clocktid, CLOCK_PRECISION/10);
        shell_clock_update(&shellclock);
        shell_clock_display(iotid, &shellclock);
    }
    Exit();
}

static void shell_sensor_task() {
    ActiveTrainSensorList active_list; active_list.size = 0;
    ActiveTrainSensorList output_list; output_list.size = 0;

    int clocktid = WhoIs(CLOCK_SERVER_NAME);
    int iotid = WhoIs(IO_SERVER_NAME);
    int traintid = WhoIs(TRAINSET_SERVER_NAME);

    for (;;) {
        Delay(clocktid, SENSOR_READ_INTERVAL/10);
        active_list = Trainset_Sensor_Readall(traintid);
        for (uint32_t i = 0; i < active_list.size; i++) {
            if (output_list.size < SENSOR_LIST_SIZE) {
                output_list.size += 1;
            }
            for (int t = SENSOR_LIST_SIZE - 1; t > 0; t--) {
                output_list.sensors[t] = output_list.sensors[t - 1];
            }
            output_list.sensors[0] = active_list.sensors[i];
        }
        for (uint32_t i = 0; i < output_list.size; i++) {
            Printf(iotid, COM2,
                "\033[%d;%dH\033[K%c%u",
                LINE_SENSOR_START + i, 3,
                output_list.sensors[i].module,
                output_list.sensors[i].id
            );
        };
    }
    Exit();
}

void shell_server_root_task() {
    int iotid = WhoIs(IO_SERVER_NAME);
    // Clear the screen
    Printf(iotid, COM2, "\033[2J");
    // Hide the cursor
    Printf(iotid, COM2, "\033[?25l");

    Create(SHELL_PRIORITY, &shell_keyboard_task);
    Create(SHELL_PRIORITY, &shell_clock_task);
    Create(SHELL_PRIORITY, &shell_sensor_task);
    Exit();
}

int CreateShellServer(uint32_t priority) {
    return Create(priority, &shell_server_root_task);
}
