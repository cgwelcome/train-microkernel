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

void trainmanager_dispatch_action(TMRequest *request) {
    switch(request->type) {
        case TM_REQUEST_INIT_TRACK:
            trainmanager_init_track(request->arg1);
            break;
        case TM_REQUEST_SPEED:
            trainmanager_speed(request->arg1, request->arg2);
            break;
        case TM_REQUEST_REVERSE:
            trainmanager_reverse(request->arg1);
            break;
        case TM_REQUEST_MOVE:
            trainmanager_move(request->arg1, request->arg2, request->arg3, request->arg4);
            break;
        case TM_REQUEST_SWITCH_ALL:
            trainmanager_switch_all(request->arg1);
            break;
        case TM_REQUEST_SWITCH_ONE:
            trainmanager_switch_one(request->arg1, request->arg2);
            break;
        case TM_REQUEST_SWITCH_DONE:
            trainmanager_switch_done();
            break;
        case TM_REQUEST_UPDATE_STATUS:
            trainmanager_update_status();
            break;
        case TM_REQUEST_PARK:
            trainmanager_park(request->arg1);
            break;
        case TM_REQUEST_STOP:
            trainmanager_stop();
            break;
        case TM_REQUEST_DONE:
            trainmanager_done();
            break;
        default:
            throw("unknown request");
    }
}

static void trainmanager_root_task() {
    int tid;
    TMRequest request;

    RegisterAs(SERVER_NAME_TMS);
    trainmanager_init();
    for (;;) {
        Receive(&tid, (char *)&request, sizeof(request));
        switch(request.type) {
            case TM_REQUEST_INIT_JOB:
                trainmanager_init_job(tid);
                break;
            default:
                trainmanager_dispatch_action(&request);
                Reply(tid, NULL, 0);
                break;
        }
    }
}

void trainnotifier_sensor_task() {
	int clocktid = WhoIs(SERVER_NAME_CLOCK);
	int traintid = WhoIs(SERVER_NAME_TMS);

    TMRequest request = {
        .type = TM_REQUEST_UPDATE_STATUS,
    };

	for (;;) {
		Delay(clocktid, SENSOR_READ_INTERVAL);
		assert(Send(traintid, (char *)&request, sizeof(request), NULL, 0) >= 0);
	}
	Exit();
}

void CreateTrainManagerServer() {
    Create(PRIORITY_SERVER_TMS, &trainmanager_root_task);
	Create(PRIORITY_NOTIFIER_TMS_SENSORS, &trainnotifier_sensor_task);
}