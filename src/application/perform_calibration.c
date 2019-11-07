#include <server/idle.h>
#include <server/io.h>
#include <server/name.h>
#include <server/clock.h>
#include <server/scheduler.h>
#include <server/train.h>
#include <server/ui.h>
#include <user/tasks.h>

#include <user/io.h>
#include <user/name.h>
#include <user/tasks.h>
#include <user/train.h>

void train_velocity_test() {
    int servertid = WhoIs(SERVER_NAME_TMS);
    int iotid = WhoIs(SERVER_NAME_IO);
    uint32_t speed = 10;
    uint32_t velocity = 1;

	TrainInitTrack(servertid, TRAIN_TRACK_A);
	/*Printf(iotid, COM2, "Track: %c\n\r", track);*/
	TrainSwitchOne(servertid, 6, TRAIN_SWITCH_STRAIGHT);
	TrainSwitchOne(servertid, 9, TRAIN_SWITCH_STRAIGHT);
	TrainSwitchOne(servertid, 15, TRAIN_SWITCH_STRAIGHT);
	uint32_t train = 78;
    for (;;) {
        int command = Getc(iotid, COM2);
        if (command == 'w' && speed < 14) {
            speed++;
            Printf(iotid, COM2, "Speed: %d\n\r", speed);
            TrainSetSpeed(servertid, train, speed);
        }
        if (command == 's' && speed > 0) {
            speed--;
            Printf(iotid, COM2, "Speed: %d\n\r", speed);
            TrainSetSpeed(servertid, train, speed);
        }
        if (command == 's' && speed > 0) {
            TrainSetSpeed(servertid, train, speed);
        }
        if (command == 'k' && speed > 0) {
            velocity++;
            Printf(iotid, COM2, "Speed: %d Velocity: %d\n\r", speed, velocity);
			TrainSetVelocity(servertid, speed, velocity);
        }
        if (command == 'j' && speed > 0) {
            velocity--;
            Printf(iotid, COM2, "Speed: %d Velocity: %d\n\r", speed, velocity);
			TrainSetVelocity(servertid, speed, velocity);
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
void train_test_velocity_test() {
    int servertid = WhoIs(SERVER_NAME_TMS);
    int iotid = WhoIs(SERVER_NAME_IO);
    uint32_t speed = 10;
    uint32_t velocity = 1;

	TrainInitTrack(servertid, TRAIN_TRACK_A);
	/*Printf(iotid, COM2, "Track: %c\n\r", track);*/
	TrainSwitchOne(servertid, 6, TRAIN_SWITCH_STRAIGHT);
	TrainSwitchOne(servertid, 9, TRAIN_SWITCH_STRAIGHT);
	TrainSwitchOne(servertid, 15, TRAIN_SWITCH_STRAIGHT);
	uint32_t train = 74;
    for (;;) {
        int command = Getc(iotid, COM2);
        if (command == 'w' && speed < 14) {
            speed++;
            Printf(iotid, COM2, "Speed: %d\n\r", speed);
            TrainSetSpeed(servertid, train, speed);
        }
        if (command == 's' && speed > 0) {
            speed--;
            Printf(iotid, COM2, "Speed: %d\n\r", speed);
            TrainSetSpeed(servertid, train, speed);
        }
        if (command == 's' && speed > 0) {
            TrainSetSpeed(servertid, train, speed);
        }
        if (command == 'k' && speed > 0) {
            velocity++;
            Printf(iotid, COM2, "Speed: %d Velocity: %d\n\r", speed, velocity);
			TrainSetVelocity(servertid, speed, velocity);
        }
        if (command == 'j' && speed > 0) {
            velocity--;
            Printf(iotid, COM2, "Speed: %d Velocity: %d\n\r", speed, velocity);
			TrainSetVelocity(servertid, speed, velocity);
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



void calibration_perform_root_task() {
    CreateNameServer();
    CreateClockServer();
    CreateIOServer();
    CreateSchedulerServer();
    CreateTrainServer();
    Create(2000, &train_velocity_test);
    CreateIdleTask();
    Exit();
}
