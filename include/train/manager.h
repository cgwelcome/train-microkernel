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

void train_manager_locate_trains(TrainSensorList *list);

void train_manager_navigate_train(uint32_t train_id, uint32_t speed, TrainSensor *sensor, int32_t offset);

#endif /*__TRAIN_MANAGER_H__*/
