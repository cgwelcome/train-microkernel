#include <user/trainmanager.h>
#include <user/ipc.h>
#include <utils/assert.h>

void TrainManager_InitTrack(int tid, TrainTrackType type) {
    TMRequest request = {
        .type = TM_REQUEST_INIT_TRACK,
        .arg1 = type,
    };
    assert(Send(tid, (char *)&request, sizeof(request), NULL, 0) >= 0);
}

void TrainManager_Speed(int tid, uint32_t train_id, uint32_t speed) {
    TMRequest request = {
        .type = TM_REQUEST_SPEED,
        .arg1 = train_id,
        .arg2 = speed,
    };
    assert(Send(tid, (char *)&request, sizeof(request), NULL, 0) >= 0);
}

void TrainManager_Reverse(int tid, uint32_t train_id) {
    TMRequest request = {
        .type = TM_REQUEST_REVERSE,
        .arg1 = train_id,
    };
    assert(Send(tid, (char *)&request, sizeof(request), NULL, 0) >= 0);
}

void TrainManager_Move(int tid, uint32_t train_id, uint32_t speed, uint32_t node_id, int32_t offset) {
    TMRequest request = {
        .type = TM_REQUEST_REVERSE,
        .arg1 = train_id,
        .arg2 = speed,
        .arg3 = node_id,
        .arg4 = (uint32_t)offset,
    };
    assert(Send(tid, (char *)&request, sizeof(request), NULL, 0) >= 0);
}

void TrainManager_Switch_All(int tid, TrainSwitchStatus status) {
    TMRequest request = {
        .type = TM_REQUEST_SWITCH_ALL,
        .arg1 = status,
    };
    assert(Send(tid, (char *)&request, sizeof(request), NULL, 0) >= 0);
}

void TrainManager_Switch_One(int tid, uint32_t switch_id, TrainSwitchStatus status) {
    TMRequest request = {
        .type = TM_REQUEST_SWITCH_ONE,
        .arg1 = switch_id,
        .arg2 = status,
    };
    assert(Send(tid, (char *)&request, sizeof(request), NULL, 0) >= 0);
}

void TrainManager_Stop(int tid) {
    TMRequest request = {
        .type = TM_REQUEST_STOP,
    };
    assert(Send(tid, (char *)&request, sizeof(request), NULL, 0) >= 0);
}

void TrainManager_Done(int tid) {
    TMRequest request = {
        .type = TM_REQUEST_DONE,
    };
    assert(Send(tid, (char *)&request, sizeof(request), NULL, 0) >= 0);
}
