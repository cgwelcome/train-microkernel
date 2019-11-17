#ifndef __TRAIN_MANAGER_H__
#define __TRAIN_MANAGER_H__

#include <stdint.h>
#include <train/train.h>

#define MAX_SENSOR_LOG 5

typedef struct {
    TrainSensor sensor;
    Train *train;
    uint32_t error;
} SensorAttribution;

typedef struct {
    SensorAttribution attributions[MAX_SENSOR_LOG];
    uint32_t size;
} SensorAttributionList;

void train_manager_init(int tid);

void train_manager_locate_trains(TrainSensorList *list);

#endif /*__TRAIN_MANAGER_H__*/
