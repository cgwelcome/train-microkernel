#ifndef __TRAIN_MANAGER_H__
#define __TRAIN_MANAGER_H__

#include <stdint.h>
#include <train/train.h>

void train_manager_initialize_train(Train *train);

void train_manager_navigate_train(Train *train, TrackNode *dest, int32_t offset);

void train_manager_issue_directives();

bool train_manager_unblocked_train(Train *train);

bool train_manager_will_collide_train(Train *train);

bool train_manager_will_arrive_final(Train *train);

bool train_manager_will_arrive_reverse(Train *train);

#endif /*__TRAIN_MANAGER_H__*/
