#ifndef __TRAIN_TRAINSET_H__
#define __TRAIN_TRAINSET_H__

#include <stdint.h>

#define TRAINSET_GO      96
#define TRAINSET_STOP    97

#define TRAIN_REVERSE    15
#define TRAIN_LIGHT      16

#define TRAINSWITCH_STRAIGHT     33
#define TRAINSWITCH_CURVED       34
#define TRAINSWITCH_DONE         32

#define TRAINSENSOR_ALL        0x80
#define TRAINSENSOR_ONE        0xC0

#define MODULE_TOTAL_NUM          5
#define MAX_SENSOR_PER_MODULE    16
#define MAX_SENSOR_NUM           80

typedef struct {
    int tid;
    int uart;
} TrainIO;

// We have TrainSwitch -> TrainSensor
typedef struct {
    uint32_t id;
    char module;
} TrainSensor;

typedef struct {
    TrainSensor sensors[MAX_SENSOR_NUM];
    uint32_t size;
} ActiveTrainSensorList;

void trainset_go(TrainIO *io);

void trainset_stop(TrainIO *io);

void trainset_speed(TrainIO *io, uint32_t train_id, uint32_t speed);

void trainset_reverse(TrainIO *io, uint32_t train_id, uint32_t speed);

void trainset_switch(TrainIO *io, uint32_t switch_id, uint32_t status);

void trainset_switch_done(TrainIO *io);

ActiveTrainSensorList trainset_sensor_readall(TrainIO *io);

#endif /*__TRAIN_TRAINSET_H__*/
