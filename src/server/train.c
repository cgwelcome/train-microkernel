#include <kernel.h>
#include <server/clock.h>
#include <server/train.h>
#include <train/manager.h>
#include <user/clock.h>
#include <user/tasks.h>
#include <user/ipc.h>
#include <user/name.h>
#include <utils/assert.h>
#include <utils/bwio.h>

static void train_manager_root_task() {
    int tid;
    TrainRequest request;

    RegisterAs(SERVER_NAME_TMS);
    track_instance()->inited = false;
    trainmanager_init();
    for (;;) {
        Receive(&tid, (char *)&request, sizeof(request));
        switch(request.type) {
            case TRAIN_REQUEST_UPDATE_STATUS:
                trainmanager_update_status();
                break;
            case TRAIN_REQUEST_INIT_TRACK:
                trainmanager_init_track(request.arg1);
                break;
            case TRAIN_REQUEST_START:
                trainmanager_start(request.arg1);
                break;
            case TRAIN_REQUEST_SPEED:
                trainmanager_speed(request.arg1, request.arg2);
                break;
            case TRAIN_REQUEST_REVERSE:
                trainmanager_reverse(request.arg1);
                break;
            case TRAIN_REQUEST_MOVE:
                // trainmanager_move(request.arg1, request.arg2, request.arg3, request.arg4);
                break;
            case TRAIN_REQUEST_SWITCH_ALL:
                trainmanager_switch_all((uint8_t)request.arg1);
                break;
            case TRAIN_REQUEST_SWITCH_ONE:
                trainmanager_switch_one(request.arg1, (uint8_t)request.arg2);
                break;
            case TRAIN_REQUEST_SWITCH_DONE:
                trainmanager_switch_done();
                break;
            case TRAIN_REQUEST_PARK:
                trainmanager_park(request.arg1);
                break;
            case TRAIN_REQUEST_STOP:
                trainmanager_stop();
                break;
            case TRAIN_REQUEST_EXIT:
                trainmanager_done();
                break;
            default:
                throw("unknown request");
        }
        Reply(tid, NULL, 0);
    }
}

void train_sensor_notifier_task() {
	int clocktid = WhoIs(SERVER_NAME_CLOCK);
	int traintid = WhoIs(SERVER_NAME_TMS);

    TrainRequest request = {
        .type = TRAIN_REQUEST_UPDATE_STATUS,
    };

	for (;;) {
		Delay(clocktid, SENSOR_READ_INTERVAL);
		assert(Send(traintid, (char *)&request, sizeof(request), NULL, 0) >= 0);
	}
	Exit();
}

void CreateTrainServer() {
    Create(PRIORITY_SERVER_TMS, &train_manager_root_task);
    Create(PRIORITY_NOTIFIER_TMS_SENSORS, &train_sensor_notifier_task);
}
