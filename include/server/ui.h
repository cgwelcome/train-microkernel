#ifndef __SERVER_UI_H__
#define __SERVER_UI_H__

#include <stdint.h>

/**
 * @defgroup ui
 * @ingroup server
 *
 * @{
 */

#define CLOCK_PRECISION 100

#define LINE_TIME         2
#define LINE_SWITCH_TITLE 3
#define LINE_SWITCH_START 4
#define LINE_SENSOR_TITLE 8
#define LINE_SENSOR_START 9
#define LINE_TERMINAL    16
#define LINE_DEBUG       17

#define CMD_BUFFER_SIZE  24
#define SENSOR_LIST_SIZE 7

typedef struct {
    uint32_t minute;
    uint32_t second;
    uint32_t decisecond;
} UIClock;

/**
 * Creates a UI Server
 */
void CreateUIServer();

/** @}*/

#endif /*__SERVER_UI_H__*/
