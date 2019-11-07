#ifndef __USER_TRAIN_H__
#define __USER_TRAIN_H__

#include <stdint.h>
#include <server/train.h>

void TrainInitTrack(int tid, TrainTrackName name);

void TrainSetSpeed(int tid, uint32_t train_id, uint32_t speed);

void TrainReverse(int tid, uint32_t train_id);

void TrainMove(int tid, uint32_t train_id, uint32_t speed, uint32_t node_id, int32_t offset);

void TrainSwitchAll(int tid, TrainSwitchStatus status);

void TrainSwitchOne(int tid, uint32_t switch_id, TrainSwitchStatus status);

void TrainSetVelocity(int tid, uint32_t speed, uint32_t velocity);

void TrainStop(int tid);

void TrainExit(int tid);

#endif /*__USER_TRAIN_H__*/
