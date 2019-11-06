#ifndef __USER_TRAINMANAGER_H__
#define __USER_TRAINMANAGER_H__

#include <stdint.h>
#include <server/train.h>

void TrainManager_Init_Track(int tid, TrainTrackType type);

void TrainManager_Speed(int tid, uint32_t train_id, uint32_t speed);

void TrainManager_Reverse(int tid, uint32_t train_id);

void TrainManager_Move(int tid, uint32_t train_id, uint32_t speed, uint32_t node_id, int32_t offset);

void TrainManager_Switch_All(int tid, TrainSwitchStatus status);

void TrainManager_Switch_One(int tid, uint32_t switch_id, TrainSwitchStatus status);

void TrainManager_Stop(int tid);

void TrainManager_Done(int tid);

#endif /*__USER_TRAINMANAGER_H__*/
