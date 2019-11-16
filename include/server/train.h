#ifndef __SERVER_TRAIN_H__
#define __SERVER_TRAIN_H__

#include <stdint.h>

typedef enum {
    TRAIN_REQUEST_INIT_TRACK,
    TRAIN_REQUEST_INIT_TRAIN,
    TRAIN_REQUEST_WAKE_CONTROLLER,
    TRAIN_REQUEST_LOCATE_TRAINS,
    TRAIN_REQUEST_SPEED,
    TRAIN_REQUEST_REVERSE,
    TRAIN_REQUEST_MOVE,
    TRAIN_REQUEST_SWITCH,
    TRAIN_REQUEST_EXIT,
} TrainRequestType;

typedef struct {
    TrainRequestType type;
    uint32_t         args[4];
} TrainRequest;

/**
 * Create Train Manager Sever responsible at tracking trains state,
 * which are not detectable from the Trainset
 */
void CreateTrainServer();

#endif /*__SERVER_TRAIN_H__*/
