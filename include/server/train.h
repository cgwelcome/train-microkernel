#ifndef __SERVER_TRAIN_H__
#define __SERVER_TRAIN_H__

#include <stdint.h>

#define SENSOR_READ_INTERVAL 5

typedef enum {
    TRAINTRACKTYPE_A,
    TRAINTRACKTYPE_B,
} TrainTrackType;

typedef enum {
    TRAINSWITCHSTATUS_STRAIGHT,
    TRAINSWITCHSTATUS_CURVED,
} TrainSwitchStatus;

typedef enum {
    TM_REQUEST_INIT_TRACK,
    TM_REQUEST_SPEED,
    TM_REQUEST_REVERSE,
    TM_REQUEST_MOVE,
    TM_REQUEST_SWITCH_ALL,
    TM_REQUEST_SWITCH_ONE,
    TM_REQUEST_SWITCH_DONE,
    TM_REQUEST_UPDATE_STATUS,
    TM_REQUEST_INIT_JOB,
    TM_REQUEST_PARK,
    TM_REQUEST_STOP,
    TM_REQUEST_DONE,
} TMRequestType;

typedef struct {
    TMRequestType type;
    uint32_t arg1;
    uint32_t arg2;
    uint32_t arg3;
    uint32_t arg4;
} TMRequest;

/**
 * Create Train Manager Sever responsible at tracking trains state,
 * which are not detectable from the Trainset
 */
void CreateTrainManagerServer();

#endif /*__SERVER_TRAIN_H__*/
