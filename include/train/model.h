#ifndef __TRAIN_MODEL_H__
#define __TRAIN_MODEL_H__

#include <stdint.h>
#include <train/train.h>

void model_estimate_train_status(Train *train);

void model_correct_train_status(TrainSensorList *sensorlist);

uint32_t model_estimate_train_stop_distance(uint32_t train_id, uint32_t velocity);

#endif /*__TRAIN_MODEL_H__*/
