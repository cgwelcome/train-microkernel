#include <user/train.h>
#include <user/ipc.h>
#include <utils/assert.h>

void TrainInitTrack(int tid, TrainTrackName name) {
    TrainRequest request = {
        .type = TRAIN_REQUEST_INIT_TRACK,
        .arg1 = name,
    };
    assert(Send(tid, (char *)&request, sizeof(request), NULL, 0) >= 0);
}

void TrainSetSpeed(int tid, uint32_t train_id, uint32_t speed) {
    TrainRequest request = {
        .type = TRAIN_REQUEST_SPEED,
        .arg1 = train_id,
        .arg2 = speed,
    };
    assert(Send(tid, (char *)&request, sizeof(request), NULL, 0) >= 0);
}

void TrainReverse(int tid, uint32_t train_id) {
    TrainRequest request = {
        .type = TRAIN_REQUEST_REVERSE,
        .arg1 = train_id,
    };
    assert(Send(tid, (char *)&request, sizeof(request), NULL, 0) >= 0);
}

void TrainMove(int tid, uint32_t train_id, uint32_t speed, uint32_t node_id, int32_t offset) {
    TrainRequest request = {
        .type = TRAIN_REQUEST_MOVE,
        .arg1 = train_id,
        .arg2 = speed,
        .arg3 = node_id,
        .arg4 = (uint32_t)offset,
    };
    assert(Send(tid, (char *)&request, sizeof(request), NULL, 0) >= 0);
}

void TrainSwitchAll(int tid, TrainSwitchStatus status) {
    TrainRequest request = {
        .type = TRAIN_REQUEST_SWITCH_ALL,
        .arg1 = status,
    };
    assert(Send(tid, (char *)&request, sizeof(request), NULL, 0) >= 0);
}

void TrainSwitchOne(int tid, uint32_t switch_id, TrainSwitchStatus status) {
    TrainRequest request = {
        .type = TRAIN_REQUEST_SWITCH_ONE,
        .arg1 = switch_id,
        .arg2 = status,
    };
    assert(Send(tid, (char *)&request, sizeof(request), NULL, 0) >= 0);
}

void TrainSetVelocity(int tid, uint32_t speed, uint32_t velocity) {
    TrainRequest request = {
        .type = TRAIN_REQUEST_SET_VELOCITY,
        .arg1 = speed,
        .arg2 = velocity,
    };
    assert(Send(tid, (char *)&request, sizeof(request), NULL, 0) >= 0);
}

void TrainStop(int tid) {
    TrainRequest request = {
        .type = TRAIN_REQUEST_STOP,
    };
    assert(Send(tid, (char *)&request, sizeof(request), NULL, 0) >= 0);
}

void TrainExit(int tid) {
    TrainRequest request = {
        .type = TRAIN_REQUEST_EXIT,
    };
    assert(Send(tid, (char *)&request, sizeof(request), NULL, 0) >= 0);
}
