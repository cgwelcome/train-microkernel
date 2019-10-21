#include <server/idle.h>
#include <server/io.h>
#include <server/name.h>
#include <server/clock.h>
#include <server/trainset.h>
#include <user/io.h>
#include <user/name.h>
#include <user/tasks.h>
#include <user/trainset.h>

void trainset_sensor_test() {
    ActiveTrainSensorList list;
    int servertid = WhoIs(TRAINSET_SERVER_NAME);
    int iotid = WhoIs(IO_SERVER_NAME);
    for (;;) {
        Printf(iotid, COM2, "Waiting for input\n\r");
        Getc(iotid, COM2);
        list = Trainset_Sensor_Readall(servertid);
        for (uint32_t i = 0; i < list.size; i++) {
            Printf(iotid, COM2, "%c", list.sensors[i].module);
            Printf(iotid, COM2, "%d\n\r", list.sensors[i].id);
        };
    }
    Exit();
}

void trainset_switchall_test() {
    TrainSwitchStatus status;
    int servertid = WhoIs(TRAINSET_SERVER_NAME);
    int iotid = WhoIs(IO_SERVER_NAME);
    int parity = 0;
    for (;;) {
        int c = Getc(iotid, COM2);
        Putc(iotid, COM2, (char)c);
        if (parity == 1) {
            status = SWITCHSTATUS_STRAIGHT;
            Putc(iotid, COM2, 's');
        }
        else {
            status = SWITCHSTATUS_CURVED;
            Putc(iotid, COM2, 'c');
        }
        Trainset_Switchall(servertid, status);
        parity ^= 1;
    }
    Exit();
}

void trainset_multiple_speed_test() {
    int servertid = WhoIs(TRAINSET_SERVER_NAME);
    int iotid = WhoIs(IO_SERVER_NAME);
    unsigned int speed = 10;
    Trainset_Speed(servertid, 78, speed);
    for (;;) {
        int command = Getc(iotid, COM2);
        if (command == 'w' && speed < 14) {
            speed++;
            Printf(iotid, COM2, "%d", speed);
            Trainset_Speed(servertid, 78, speed);
        }
        if (command == 's' && speed > 0) {
            speed--;
            Printf(iotid, COM2, "%d", speed);
            Trainset_Speed(servertid, 78, speed);
        }
        if (command == 'r') {
            Printf(iotid, COM2, "Reversing");
            Trainset_Reverse(servertid, 78);
        }
        if (command == 'q') {
            ShutdownIOServer();
            Shutdown();
            break;
        }
    }
    Exit();
}

void trainset_single_speed_test() {
    int servertid = WhoIs(TRAINSET_SERVER_NAME);
    Trainset_Speed(servertid, 78, 14);
    Exit();
}

void trainset_test_root_task() {
    CreateNameServer(4000);
    CreateClockServer(3700);
    CreateIOServer(3500, 3500, 3500);
    CreateTrainSetServer(3000);
    Create(2000, &trainset_multiple_speed_test);
    CreateIdleTask(1);
    Exit();
}
