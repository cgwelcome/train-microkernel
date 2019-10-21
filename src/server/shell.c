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
    Exit();
}

static void shell_clock_task() {
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


void CreateShellServer(uint32_t priority) {
    Create(priority, &shell_parser_task);
    Create(priority, &shell_keyboard_task);
    Create(priority, &shell_clock_task);
    Create(priority, &shell_sensor_task);
}
