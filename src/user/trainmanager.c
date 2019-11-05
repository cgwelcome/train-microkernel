#include <stdint.h>
#include <server/trainmanager.h>
#include <user/ipc.h>

void TrainManager_InitTrack(int tid, TrainTrackType type) {
    TMRequest request = {
        .type = TMREQUESTTYPE_INIT_TRACK,
        .arg1 = type,
    };
    Send(tid, (char *)&request, sizeof(request), NULL, 0);
}

void TrainManager_Speed(int tid, uint32_t train_id, uint32_t speed) {
    TMRequest request = {
        .type = TMREQUESTTYPE_SPEED,
        .arg1 = train_id,
        .arg2 = speed,
    };
    Send(tid, (char *)&request, sizeof(request), NULL, 0);
}

void TrainManager_Reverse(int tid, uint32_t train_id) {
    TMRequest request = {
        .type = TMREQUESTTYPE_REVERSE,
        .arg1 = train_id,
    };
    Send(tid, (char *)&request, sizeof(request), NULL, 0);
}

void TrainManager_Move(int tid, uint32_t train_id, uint32_t speed, uint32_t node_id, int32_t offset) {
    TMRequest request = {
        .type = TMREQUESTTYPE_REVERSE,
        .arg1 = train_id,
        .arg2 = speed,
        .arg3 = node_id,
        .arg4 = (uint32_t)offset,
    };
    Send(tid, (char *)&request, sizeof(request), NULL, 0);
}

void TrainManager_Switch_All(int tid, TrainSwitchStatus status) {
    TMRequest request = {
        .type = TMREQUESTTYPE_SWITCH_ALL,
        .arg1 = status,
    };
    Send(tid, (char *)&request, sizeof(request), NULL, 0);
}

void TrainManager_Switch_One(int tid, uint32_t switch_id, TrainSwitchStatus status) {
    TMRequest request = {
        .type = TMREQUESTTYPE_SWITCH_ONE,
        .arg1 = switch_id,
        .arg2 = status,
    };
    Send(tid, (char *)&request, sizeof(request), NULL, 0);
}

void TrainManager_Stop(int tid) {
    TMRequest request = {
        .type = TMREQUESTTYPE_STOP,
    };
    Send(tid, (char *)&request, sizeof(request), NULL, 0);
}

void TrainManager_Done(int tid) {
    TMRequest request = {
        .type = TMREQUESTTYPE_DONE,
    };
    Send(tid, (char *)&request, sizeof(request), NULL, 0);
}
