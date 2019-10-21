#ifndef __SERVER_TRAINSET_H__
#define __SERVER_TRAINSET_H__

#include <stdint.h>

#define TRAINSET_GO      96
#define TRAINSET_STOP    97

#define TRAIN_TOTAL_NUM  80
#define TRAIN_REVERSE    15
#define TRAIN_LIGHT      16

#define SWITCH_TOTAL_NUM 22
#define SWITCH_STRAIGHT  33
#define SWITCH_CURVED    34
#define SWITCH_DONE      32

#define SENSOR_READALL        0x80
#define SENSOR_READONE        0xC0

#define SWITCH_MOVE_INTERVAL      150
#define TRAIN_REVERSE_INTERVAL   4000

#define MODULE_TOTAL_NUM          5

// MAXNUM means the highest id switch can get
#define MAX_TRAIN_NUM            80
#define MAX_SENSOR_PER_MODULE    16
#define MAX_SENSOR_NUM           80
#define MAX_SWITCH_NUM          157

#define TRAINSET_SERVER_NAME  "TS"

#define SWITCHSTATUS_DEFAULT SWITCHSTATUS_CURVED

typedef struct {
    uint32_t id;
    uint32_t speed;
} Train;

typedef struct {
    uint32_t id;
    char module;
} TrainSensor;

typedef enum {
    SWITCHSTATUS_STRAIGHT,
    SWITCHSTATUS_CURVED,
} TrainSwitchStatus;

// Named TrainSwitch instead of Switch since switch is C keyword
typedef struct {
    uint32_t id;
    TrainSwitchStatus status;
} TrainSwitch;

// TODO: Have a template queue
typedef struct {
    TrainSensor sensors[MAX_SENSOR_NUM];
    uint32_t size;
} ActiveTrainSensorList;

typedef enum {
    TSREQUESTTYPE_GO,
    TSREQUESTTYPE_STOP,
    TSREQUESTTYPE_SPEED,
    TSREQUESTTYPE_REVERSE,
    TSREQUESTTYPE_SWITCHONE,
    TSREQUESTTYPE_SWITCHALL,
    TSREQUESTTYPE_SENSOR_READALL,
    TSREQUESTTYPE_INIT_TIMEOUT,
    TSREQUESTTYPE_REVERSE_TIMEOUT,
    TSREQUESTTYPE_SWITCH_TIMEOUT,
    TSREQUESTTYPE_DONE,
} TSRequestType;

typedef struct {
    TSRequestType type;
    uint32_t arg1;
    uint32_t arg2;
} TSRequest;

typedef struct {
    TSRequest payload;
    uint32_t delay; // in milliseconds
} TSTimerResponse;

int CreateTrainSetServer(uint32_t priority);

#endif /*__SERVER_TRAINSET_H__*/
