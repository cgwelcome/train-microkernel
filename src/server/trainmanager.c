#include <train/manager.h>
#include <train/notifier.h>
#include <user/tasks.h>
#include <user/ipc.h>
#include <user/name.h>
#include <utils/bwio.h>
#include <priority.h>

void trainmanager_dispatch_action(TMRequest *request) {
    switch(request->type) {
        case TMREQUESTTYPE_INIT_TRACK:
            trainmanager_init_track(request->arg1);
            break;
        case TMREQUESTTYPE_SPEED:
            trainmanager_speed(request->arg1, request->arg2);
            break;
        case TMREQUESTTYPE_REVERSE:
            trainmanager_reverse(request->arg1);
            break;
        case TMREQUESTTYPE_MOVE:
            trainmanager_move(request->arg1, request->arg2, request->arg3, request->arg4);
            break;
        case TMREQUESTTYPE_SWITCH_ALL:
            trainmanager_switch_all(request->arg1);
            break;
        case TMREQUESTTYPE_SWITCH_ONE:
            trainmanager_switch_one(request->arg1, request->arg2);
            break;
        case TMREQUESTTYPE_SWITCH_DONE:
            trainmanager_switch_done();
            break;
        case TMREQUESTTYPE_UPDATE_STATUS:
            trainmanager_update_status();
            break;
        case TMREQUESTTYPE_PARK:
            trainmanager_park(request->arg1);
            break;
        case TMREQUESTTYPE_STOP:
            trainmanager_stop();
            break;
        case TMREQUESTTYPE_DONE:
            trainmanager_done();
            break;
        default:
            return;
            break;
    }
}

static void trainmanager_root_task() {
    int tid;
    TMRequest request;

    RegisterAs(TRAINMANAGER_SERVER_NAME);
    trainmanager_init();
    for (;;) {
        Receive(&tid, (char *)&request, sizeof(request));
        switch(request.type) {
            case TMREQUESTTYPE_INIT_TRACK:
            case TMREQUESTTYPE_SPEED:
            case TMREQUESTTYPE_REVERSE:
            case TMREQUESTTYPE_MOVE:
            case TMREQUESTTYPE_SWITCH_ALL:
            case TMREQUESTTYPE_SWITCH_ONE:
            case TMREQUESTTYPE_SWITCH_DONE:
            case TMREQUESTTYPE_UPDATE_STATUS:
            case TMREQUESTTYPE_PARK:
            case TMREQUESTTYPE_STOP:
            case TMREQUESTTYPE_DONE:
                trainmanager_dispatch_action(&request);
                Reply(tid, NULL, 0);
                break;
            case TMREQUESTTYPE_INIT_JOB:
                trainmanager_init_job(tid);
                break;
        }
    }
}

void CreateTrainManagerServer() {
    Create(PRIORITY_SERVER_TMS, &trainmanager_root_task);
	Create(PRIORITY_NOTIFIER_TMS_SENSORS, &trainnotifier_sensor_task);
}
