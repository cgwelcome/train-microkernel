#ifndef __TRAIN_CONTROLLER_H__
#define __TRAIN_CONTROLLER_H__

#include <stdint.h>
#include <train/track.h>

#define CONTROLLER_SWITCH_DONE_TIMEOUT  100 // ms
#define CONTROLLER_REVERSE_DELAY       1500
#define CONTROLLER_DIRECTIVE_LIMIT     1024

#define TRAIN_CODE_GO                 96
#define TRAIN_CODE_STOP               97
#define TRAIN_CODE_SWITCH_STRAIGHT    33
#define TRAIN_CODE_SWITCH_CURVED      34
#define TRAIN_CODE_SWITCH_DONE        32
#define TRAIN_CODE_SENSOR_ALL       0x80
#define TRAIN_CODE_SENSOR_ONE       0xC0

#define TRAIN_STATUS_REVERSE          15
#define TRAIN_STATUS_LIGHT            16

typedef enum {
    TRAIN_DIRECTIVE_NONE,
    TRAIN_DIRECTIVE_SPEED,
    TRAIN_DIRECTIVE_SWITCH,
} TrainDirectiveType;

typedef struct {
    TrainDirectiveType type;
    uint32_t id;
    uint32_t data;
} TrainDirective;

typedef struct {
    char module;
    uint32_t id;
} TrainSensor;

typedef struct {
    TrainSensor sensors[MAX_SENSOR_NUM];
    uint32_t size;
} TrainSensorList;

void controller_init();

void controller_wake();

void controller_go();

void controller_stop();

void controller_speed_one(uint32_t train_id, uint32_t speed, uint32_t delay /*in ms*/);

void controller_speed_all(uint32_t speed, uint32_t delay /*in ms*/);

void controller_switch_one(uint32_t switch_id, uint32_t direction, uint32_t delay /*in ms*/);

void controller_switch_all(uint32_t direction, uint32_t delay /*in ms*/);

void controller_read_sensors(TrainSensorList *sensorlist);

#endif /*__TRAIN_DIRECTIVE_H__*/
