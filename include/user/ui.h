#ifndef __USER_UI_H__
#define __USER_UI_H__

#include <stdint.h>
#include <kernel.h>
#include <train/trainset.h>
#include <user/io.h>

#define LINE_TIME         2
#define LINE_SWITCH_TITLE 3
#define LINE_SWITCH_START 4
#define LINE_SENSOR_TITLE 8
#define LINE_SENSOR_START 9
#define LINE_TERMINAL    16
#define LINE_DEBUG       17

void PrintBasicInterface(int io_tid);

void PrintTime(int iotid);

void PrintSwitch(int io_tid, unsigned int code, char direction);

void PrintSensors(int io_tid, TrainSensor *list, size_t size);

void PrintTerminal(int io_tid, char *cmd_buffer, unsigned int cmd_len);

#endif /*__USER_TRAIN_H__*/
