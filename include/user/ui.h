#ifndef __USER_UI_H__
#define __USER_UI_H__

#include <stdint.h>
#include <train/manager.h>
#include <train/controller.h>

#define LINE_IDLE            1
#define LINE_TIME            2
#define LINE_SWITCH_TITLE    3
#define LINE_SWITCH_START    4
#define LINE_LOCATION_TITLE  8
#define LINE_LOCATION_START  9
#define LINE_SENSOR_TITLE   15
#define LINE_SENSOR_START   16
#define LINE_SENSOR_END     21
#define LINE_TERMINAL       22
#define LINE_LOG_START      24
#define LINE_LOG_END       128

#define MAX_NUM_ARGS        10

void PrintBasicInterface(int io_tid);

void PrintIdle(int io_tid, int usage);

void PrintTime(int iotid);

void PrintSwitch(int io_tid, unsigned int code, uint8_t direction);

void PrintLocation(int io_tid, Train *train);

void PrintSensor(int io_tid, TrainSensor *sensor, uint32_t time);

void PrintTerminal(int io_tid, const char *cmd_buffer);

#endif /*__USER_TRAIN_H__*/
