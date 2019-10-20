#ifndef __USER_TRAINSET_H__
#define __USER_TRAINSET_H__

#include <stdint.h>
#include <server/trainset.h>

void Trainset_Go(int tid);

void Trainset_Stop(int tid);

void Trainset_Speed(int tid, uint32_t train_id, uint32_t speed);

void Trainset_Reverse(int tid, uint32_t train_id);

ActiveTrainSensorList Trainset_Sensor_Readall(int tid);

void Trainset_Switchone(int tid, uint32_t switch_id, TrainSwitchStatus status);

void Trainset_Switchall(int tid, TrainSwitchStatus status);

void Trainset_Done(int tid);

#endif /*__USER_TRAINSET_H__*/
