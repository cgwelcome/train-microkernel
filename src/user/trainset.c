#include <stdint.h>
#include <server/trainset.h>
#include <user/ipc.h>

void Trainset_Go(int tid) {
    TSRequest request = {
        .type = TSREQUESTTYPE_GO,
    };
    Send(tid, (char *)&request, sizeof(request), NULL, 0);
}

void Trainset_Stop(int tid) {
    TSRequest request = {
        .type = TSREQUESTTYPE_STOP,
    };
    Send(tid, (char *)&request, sizeof(request), NULL, 0);
}

void Trainset_Speed(int tid, uint32_t train_id, uint32_t speed) {
    TSRequest request = {
        .type = TSREQUESTTYPE_SPEED,
        .arg1 = train_id,
        .arg2 = speed,
    };
    Send(tid, (char *)&request, sizeof(request), NULL, 0);
}

void Trainset_Reverse(int tid, uint32_t train_id) {
    TSRequest request = {
        .type = TSREQUESTTYPE_REVERSE,
        .arg1 = train_id,
    };
    Send(tid, (char *)&request, sizeof(request), NULL, 0);
}

void Trainset_Switch(int tid, uint32_t switch_id, TrainSwitchStatus status) {
    TSRequest request = {
        .type = TSREQUESTTYPE_SWITCH,
        .arg1 = switch_id,
        .arg2 = status,
    };
    Send(tid, (char *)&request, sizeof(request), NULL, 0);
}

ActiveTrainSensorList Trainset_Sensor_Readall(int tid) {
    ActiveTrainSensorList sensorlist;
    TSRequest request = {
        .type = TSREQUESTTYPE_SENSOR_READALL,
    };
    Send(tid, (char *)&request, sizeof(request), (char *)&sensorlist, sizeof(sensorlist));
    return sensorlist;
}

void Trainset_Done(int tid) {
    TSRequest request = {
        .type = TSREQUESTTYPE_DONE,
    };
    Send(tid, (char *)&request, sizeof(request), NULL, 0);
}
