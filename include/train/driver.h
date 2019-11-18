#ifndef __TRAIN_DRIVER_H__
#define __TRAIN_DRIVER_H__

#include <stdint.h>
#include <train/track.h>

void driver_navigate(uint32_t train_id, uint32_t speed, TrainSensor *sensor, int32_t offset);

#endif /*__TRAIN_DRIVER_H__*/
