#ifndef __TRAIN_DRIVER_H__
#define __TRAIN_DRIVER_H__

#include <train/train.h>

void driver_handle_speed(Train *train, uint32_t speed);

void driver_handle_reverse(Train *train);

void driver_handle_move(Train *train, uint32_t speed);

void driver_cruise(Train *train);

void driver_brake_command(Train *train);

void driver_brake_reverse(Train *train);

void driver_brake_traffic(Train *train);

void driver_wait_command(Train *train);

void driver_wait_reverse(Train *train);

void driver_wait_traffic(Train *train);

void driver_wait_entry(Train *train);

void driver_speed_entry(Train *train);

void driver_reverse_entry(Train *train);

#endif /*__TRAIN_DRIVER_H__*/
