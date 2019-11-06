#include <kernel.h>
#include <server/idle.h>
#include <server/io.h>
#include <server/name.h>
#include <server/clock.h>
#include <server/train.h>
#include <user/io.h>
#include <user/name.h>
#include <user/tasks.h>
#include <user/train.h>

void train_switchall_test() {
    TrainSwitchStatus status;
    int servertid = WhoIs(SERVER_NAME_TMS);
    int iotid = WhoIs(SERVER_NAME_IO);
    int parity = 0;
    for (;;) {
        int c = Getc(iotid, COM2);
        Putc(iotid, COM2, (char)c);
        if (parity == 1) {
            status = TRAIN_SWITCH_STRAIGHT;
            Putc(iotid, COM2, 's');
        }
        else {
            status = TRAIN_SWITCH_CURVED;
            Putc(iotid, COM2, 'c');
        }
        TrainSwitchAll(servertid, status);
        parity ^= 1;
    }
    Exit();
}

void train_multiple_speed_test() {
    int servertid = WhoIs(SERVER_NAME_TMS);
    int iotid = WhoIs(SERVER_NAME_IO);
    unsigned int speed = 10;
    TrainSetSpeed(servertid, 78, speed);
    for (;;) {
        int command = Getc(iotid, COM2);
        if (command == 'w' && speed < 14) {
            speed++;
            Printf(iotid, COM2, "%d", speed);
            TrainSetSpeed(servertid, 78, speed);
        }
        if (command == 's' && speed > 0) {
            speed--;
            Printf(iotid, COM2, "%d", speed);
            TrainSetSpeed(servertid, 78, speed);
        }
        if (command == 'r') {
            Printf(iotid, COM2, "Reversing");
            TrainReverse(servertid, 78);
        }
        if (command == 'q') {
            TrainExit(servertid);
            ShutdownIOServer();
            Shutdown();
            break;
        }
    }
    Exit();
}

void train_single_speed_test() {
    int servertid = WhoIs(SERVER_NAME_TMS);
    TrainSetSpeed(servertid, 78, 14);
    Exit();
}

void train_test_root_task() {
    CreateNameServer(4000);
    CreateClockServer(3700);
    CreateIOServer(3500, 3500, 3500);
    CreateTrainServer(3000);
    Create(2000, &train_switchall_test);
    CreateIdleTask(1);
    Exit();
}