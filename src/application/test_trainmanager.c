#include <server/idle.h>
#include <server/io.h>
#include <server/name.h>
#include <server/clock.h>
#include <server/trainmanager.h>
#include <user/io.h>
#include <user/name.h>
#include <user/tasks.h>
#include <user/trainmanager.h>

void trainmanager_switchall_test() {
    TrainSwitchStatus status;
    int servertid = WhoIs(TRAINMANAGER_SERVER_NAME);
    int iotid = WhoIs(IO_SERVER_NAME);
    int parity = 0;
    for (;;) {
        int c = Getc(iotid, COM2);
        Putc(iotid, COM2, (char)c);
        if (parity == 1) {
            status = TRAINSWITCHSTATUS_STRAIGHT;
            Putc(iotid, COM2, 's');
        }
        else {
            status = TRAINSWITCHSTATUS_CURVED;
            Putc(iotid, COM2, 'c');
        }
        TrainManager_Switch_All(servertid, status);
        parity ^= 1;
    }
    Exit();
}

void trainmanager_multiple_speed_test() {
    int servertid = WhoIs(TRAINMANAGER_SERVER_NAME);
    int iotid = WhoIs(IO_SERVER_NAME);
    unsigned int speed = 10;
    TrainManager_Speed(servertid, 78, speed);
    for (;;) {
        int command = Getc(iotid, COM2);
        if (command == 'w' && speed < 14) {
            speed++;
            Printf(iotid, COM2, "%d", speed);
            TrainManager_Speed(servertid, 78, speed);
        }
        if (command == 's' && speed > 0) {
            speed--;
            Printf(iotid, COM2, "%d", speed);
            TrainManager_Speed(servertid, 78, speed);
        }
        if (command == 'r') {
            Printf(iotid, COM2, "Reversing");
            TrainManager_Reverse(servertid, 78);
        }
        if (command == 'q') {
            TrainManager_Done(servertid);
            ShutdownIOServer();
            Shutdown();
            break;
        }
    }
    Exit();
}

void trainmanager_single_speed_test() {
    int servertid = WhoIs(TRAINMANAGER_SERVER_NAME);
    TrainManager_Speed(servertid, 78, 14);
    Exit();
}

void trainmanager_test_root_task() {
    CreateNameServer(4000);
    CreateClockServer(3700);
    CreateIOServer(3500, 3500, 3500);
    CreateTrainManagerServer(3000);
    Create(2000, &trainmanager_switchall_test);
    CreateIdleTask(1);
    Exit();
}
