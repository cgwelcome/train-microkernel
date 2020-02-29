#ifndef __USER_TRAIN_H__
#define __USER_TRAIN_H__

#include <stdint.h>
#include <server/train.h>
#include <train/track.h>

void TrainInitTrack(int tid, TrackName name);

void TrainInitTrain(int tid, uint32_t train_id, TrackNode *node);

void TrainSpeed(int tid, uint32_t train_id, uint32_t speed);

void TrainReverse(int tid, uint32_t train_id);

void TrainMove(int tid, uint32_t train_id, uint32_t speed, TrackNode *node, int32_t offset);

void TrainSwitch(int tid, uint32_t switch_id, int8_t status);

void TrainLight(int tid, bool turn_on);

uint32_t TrainFindIdle(int tid);

void TrainExit(int tid);

#endif /*__USER_TRAIN_H__*/
