#ifndef __USER_UI_H__
#define __USER_UI_H__

#include <stdint.h>
#include <kernel.h>
#include <train/track.h>
#include <user/io.h>

#define LINE_TIME            2
#define LINE_SWITCH_TITLE    3
#define LINE_SWITCH_START    4
#define LINE_LOCATION_TITLE  8
#define LINE_LOCATION_START  9
#define LINE_TERMINAL       16
#define LINE_DEBUG          17
#define MAX_NUM_ARGS        10

void PrintBasicInterface(int io_tid);

void PrintTime(int iotid);

void PrintSwitch(int io_tid, unsigned int code, char direction);

void PrintVelocity(int io_tid, uint32_t train_id, uint32_t time, uint32_t velocity);

void PrintTimeDifference(int io_tid, uint32_t train_id, uint64_t expected_time);

void PrintTerminal(int io_tid, char *cmd_buffer, unsigned int cmd_len);

#endif /*__USER_TRAIN_H__*/
