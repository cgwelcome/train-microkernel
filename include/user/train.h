#ifndef __USER_TRAIN_H__
#define __USER_TRAIN_H__

#include <stdint.h>
#include <server/train.h>
#include <train/track.h>

void TrainInitTrack(int tid, TrackName name);

void TrainStart(int tid, uint32_t train_id);

void TrainSetSpeed(int tid, uint32_t train_id, uint32_t speed);

void TrainReverse(int tid, uint32_t train_id);

void TrainMove(int tid, uint32_t train_id, uint32_t speed, uint32_t node_id, int32_t offset);

void TrainSwitchAll(int tid, int8_t status);

void TrainSwitchOne(int tid, uint32_t switch_id, int8_t status);

void TrainStop(int tid);

void TrainExit(int tid);

#endif /*__USER_TRAIN_H__*/
