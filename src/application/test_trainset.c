#include <server/idle.h>
#include <server/io.h>
#include <server/name.h>
#include <server/clock.h>
#include <server/trainset.h>
#include <user/io.h>
#include <user/name.h>
#include <user/tasks.h>
#include <user/trainset.h>

void trainset_switch_test() {
    TrainSwitchStatus status;
    int servertid = WhoIs(TRAINSET_SERVER_NAME);
    int iotid = WhoIs(IO_SERVER_NAME);
    int parity = 0;
    for (;;) {
        Getc(iotid, COM2);
        if (parity == 1) {
            status = TSWITCHSTATUS_STRAIGHT;
        }
        else {
            status = TSWITCHSTATUS_CURVED;
        }
        for (uint32_t id = 1; id <= 18; id++) {
            Trainset_Switch(servertid, id, status);
        }
        for (uint32_t id = 0x99; id <= 0x9C; id++) {
            Trainset_Switch(servertid, id, status);
        }
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
            Trainset_Done(servertid);
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
    Create(2000, &trainset_switch_test);
    CreateIdleTask(1);
    Exit();
}
