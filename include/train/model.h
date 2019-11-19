#ifndef __TRAIN_MODEL_H__
#define __TRAIN_MODEL_H__

#include <stdint.h>
#include <train/train.h>

void model_estimate_train_status(Train *train);

uint32_t model_estimate_train_stop_distance(Train *train);

#endif /*__TRAIN_MODEL_H__*/
