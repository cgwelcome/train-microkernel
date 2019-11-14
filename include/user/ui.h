#ifndef __USER_UI_H__
#define __USER_UI_H__

#include <stdint.h>
#include <kernel.h>
#include <train/track.h>
#include <user/io.h>

#define LINE_IDLE            1
#define LINE_TIME            2
#define LINE_SWITCH_TITLE    3
#define LINE_SWITCH_START    4
#define LINE_LOCATION_TITLE  8
#define LINE_LOCATION_START  9
#define LINE_TERMINAL       16
#define LINE_LOAD           18
#define MAX_NUM_ARGS        10

void PrintBasicInterface(int io_tid);

void PrintIdle(int io_tid, int usage);

void PrintTime(int iotid);

void PrintSwitch(int io_tid, unsigned int code, char direction);

void PrintVelocity(int io_tid, uint32_t train_id, uint32_t time, uint32_t velocity);

void PrintTimeDifference(int io_tid, uint32_t train_id, uint64_t expected_time);

void PrintTerminal(int io_tid, const char *cmd_buffer);

#endif /*__USER_TRAIN_H__*/
