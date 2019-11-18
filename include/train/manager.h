#ifndef __TRAIN_MANAGER_H__
#define __TRAIN_MANAGER_H__

#include <stdint.h>
#include <train/train.h>

#define MAX_SENSOR_LOG 5

typedef struct {
    TrackNode *node;
    Train *train;
    int32_t error;
} SensorAttribution;

typedef struct {
    SensorAttribution attributions[MAX_SENSOR_LOG];
    uint32_t size;
} SensorAttributionList;

void train_manager_init(int tid);

void train_manager_initialize_train(Train *train);

void train_manager_locate_trains(Train *trains, Track *track, TrainSensorList *list);

#endif /*__TRAIN_MANAGER_H__*/
