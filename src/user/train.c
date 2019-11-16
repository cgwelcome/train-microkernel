#include <user/train.h>
#include <user/ipc.h>
#include <utils/assert.h>

void TrainInitTrack(int tid, TrackName name) {
    TrainRequest request = {
        .type    = TRAIN_REQUEST_INIT_TRACK,
        .args[0] = name,
    };
    assert(Send(tid, (char *)&request, sizeof(request), NULL, 0) >= 0);
}

void TrainInitTrain(int tid, uint32_t train_id) {
    TrainRequest request = {
        .type    = TRAIN_REQUEST_INIT_TRAIN,
        .args[0] = train_id,
    };
    assert(Send(tid, (char *)&request, sizeof(request), NULL, 0) >= 0);
}

void TrainSpeed(int tid, uint32_t train_id, uint32_t speed) {
    TrainRequest request = {
        .type    = TRAIN_REQUEST_SPEED,
        .args[0] = train_id,
        .args[1] = speed,
    };
    assert(Send(tid, (char *)&request, sizeof(request), NULL, 0) >= 0);
}

void TrainReverse(int tid, uint32_t train_id) {
    TrainRequest request = {
        .type    = TRAIN_REQUEST_REVERSE,
        .args[0] = train_id,
    };
    assert(Send(tid, (char *)&request, sizeof(request), NULL, 0) >= 0);
}

void TrainMove(int tid, uint32_t train_id, uint32_t speed, uint32_t node_id, int32_t offset) {
    TrainRequest request = {
        .type    = TRAIN_REQUEST_MOVE,
        .args[0] = train_id,
        .args[1] = speed,
        .args[2] = node_id,
        .args[3] = (uint32_t) offset,
    };
    assert(Send(tid, (char *)&request, sizeof(request), NULL, 0) >= 0);
}

void TrainSwitch(int tid, uint32_t switch_id, int8_t status) {
    TrainRequest request = {
        .type    = TRAIN_REQUEST_MOVE,
        .args[0] = switch_id,
        .args[1] = (uint32_t) status,
    };
    assert(Send(tid, (char *)&request, sizeof(request), NULL, 0) >= 0);
}

void TrainExit(int tid) {
    TrainRequest request = {
        .type = TRAIN_REQUEST_EXIT,
    };
    assert(Send(tid, (char *)&request, sizeof(request), NULL, 0) >= 0);
}
