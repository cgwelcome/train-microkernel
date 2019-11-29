#ifndef __TRAIN_DRIVER_H__
#define __TRAIN_DRIVER_H__

#include <train/train.h>

typedef enum {
    DRIVER_REQUEST_SPEED,
    DRIVER_REQUEST_REVERSE,
} DriverRequestType;

typedef struct {
    DriverRequestType type;
    uint32_t speed;
} DriverRequest;

void driver_transition(Train *train, TrainState state);

void driver_cruise(Train *train, DriverRequest *request);

void driver_brake_command(Train *train, DriverRequest *request);

void driver_brake_reverse(Train *train, DriverRequest *request);

void driver_brake_traffic(Train *train, DriverRequest *request);

void driver_wait_command(Train *train, DriverRequest *request);

void driver_wait_reverse(Train *train, DriverRequest *request);

void driver_wait_traffic(Train *train, DriverRequest *request);

void driver_speed_entry(Train *train);

void driver_reverse_entry(Train *train);

#endif /*__TRAIN_DRIVER_H__*/
