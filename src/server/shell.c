#include <server/clock.h>
#include <server/io.h>
#include <server/shell.h>
#include <server/trainset.h>
#include <user/io.h>
#include <user/clock.h>
#include <user/name.h>
#include <user/tasks.h>
#include <user/trainset.h>

static void shell_parser_task() {
    Exit();
}

static void shell_keyboard_task() {
    int iotid = WhoIs(IO_SERVER_NAME);
    for (;;) {
        int c = Getc(iotid, COM2);
        Putc(iotid, COM2, (char)c);
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
    Printf(iotid, COM2, "%d:%d:%d\n\r", shellclock->minute, shellclock->second, shellclock->decisecond);
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
    ActiveTrainSensorList list;
    int clocktid = WhoIs(CLOCK_SERVER_NAME);
    int iotid = WhoIs(IO_SERVER_NAME);
    int traintid = WhoIs(TRAINSET_SERVER_NAME);
    for (;;) {
        Delay(clocktid, SENSOR_READ_INTERVAL/10);
        list = Trainset_Sensor_Readall(traintid);
        for (uint32_t i = 0; i < list.size; i++) {
            Printf(iotid, COM2, "%c", list.sensors[i].module);
            Printf(iotid, COM2, "%d\n\r", list.sensors[i].id);
        };
    }
    Exit();
}

void shell_server_root_task() {
    Create(SHELL_PRIORITY, &shell_parser_task);
    Create(SHELL_PRIORITY, &shell_keyboard_task);
    Create(SHELL_PRIORITY, &shell_clock_task);
    Create(SHELL_PRIORITY, &shell_sensor_task);
    Exit();
}

int CreateShellServer(uint32_t priority) {
    return Create(priority, &shell_server_root_task);
}
