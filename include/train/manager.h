#ifndef __TRAIN_MANAGER_H__
#define __TRAIN_MANAGER_H__

#include <stdint.h>
#include <train/train.h>

void train_manager_initialize_train(Train *train);

void train_manager_navigate_train(uint32_t train_id, TrackNode *dest, int32_t offset);

void train_manager_issue_directives();

#endif /*__TRAIN_MANAGER_H__*/
