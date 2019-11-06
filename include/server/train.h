#ifndef __SERVER_TRAIN_H__
#define __SERVER_TRAIN_H__

#include <stdint.h>

#define SENSOR_READ_INTERVAL 5

typedef enum {
    TRAIN_TRACK_A,
    TRAIN_TRACK_B,
} TrainTrackName;

typedef enum {
    TRAIN_SWITCH_STRAIGHT,
    TRAIN_SWITCH_CURVED,
} TrainSwitchStatus;

typedef enum {
    TRAIN_REQUEST_INIT_TRACK,
    TRAIN_REQUEST_SPEED,
    TRAIN_REQUEST_REVERSE,
    TRAIN_REQUEST_MOVE,
    TRAIN_REQUEST_SWITCH_ALL,
    TRAIN_REQUEST_SWITCH_ONE,
    TRAIN_REQUEST_SWITCH_DONE,
    TRAIN_REQUEST_UPDATE_STATUS,
    TRAIN_REQUEST_PARK,
    TRAIN_REQUEST_STOP,
    TRAIN_REQUEST_EXIT,
} TrainRequestType;

typedef struct {
    TrainRequestType type;
    uint32_t arg1;
    uint32_t arg2;
    uint32_t arg3;
    uint32_t arg4;
} TrainRequest;

/**
 * Create Train Manager Sever responsible at tracking trains state,
 * which are not detectable from the Trainset
 */
void CreateTrainServer();

#endif /*__SERVER_TRAIN_H__*/
