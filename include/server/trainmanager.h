#ifndef __SERVER_TRAINMANAGER_H__
#define __SERVER_TRAINMANAGER_H__

#include <stdint.h>

#define TRAINMANAGER_SERVER_NAME "TM"

typedef enum {
    TRAINTRACKTYPE_A,
    TRAINTRACKTYPE_B,
} TrainTrackType;
typedef enum {
    TRAINSWITCHSTATUS_STRAIGHT,
    TRAINSWITCHSTATUS_CURVED,
} TrainSwitchStatus;

typedef enum {
    TMREQUESTTYPE_INIT_TRACK,
    TMREQUESTTYPE_SPEED,
    TMREQUESTTYPE_REVERSE,
    TMREQUESTTYPE_MOVE,
    TMREQUESTTYPE_SWITCH_ALL,
    TMREQUESTTYPE_SWITCH_ONE,
    TMREQUESTTYPE_SWITCH_DONE,
    TMREQUESTTYPE_UPDATE_STATUS,
    TMREQUESTTYPE_INIT_JOB,
    TMREQUESTTYPE_PARK,
    TMREQUESTTYPE_STOP,
    TMREQUESTTYPE_DONE,
} TMRequestType;

typedef struct {
    TMRequestType type;
    uint32_t arg1;
    uint32_t arg2;
    uint32_t arg3;
    uint32_t arg4;
} TMRequest;

void trainmanager_dispatch_action(TMRequest *request);

/**
 * Create Train Manager Sever responsible at tracking trains state,
 * which are not detectable from the Trainset
 */
void CreateTrainManagerServer();

#endif /*__SERVER_TRAINMANAGER_H__*/
