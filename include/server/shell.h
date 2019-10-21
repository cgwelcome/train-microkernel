#ifndef __SERVER_SHELL_H__
#define __SERVER_SHELL_H__

#define SENSOR_READ_INTERVAL 100
#define CLOCK_PRECISION 100

#define SHELL_PRIORITY 3000

#include <stdint.h>

#define LINE_TIME         2
#define LINE_SWITCH_TITLE 3
#define LINE_SWITCH_START 5
#define LINE_SENSOR_TITLE 8
#define LINE_SENSOR_START 9
#define LINE_TERMINAL    16
#define LINE_DEBUG       17
#define CMD_BUFFER_SIZE  24

typedef struct {
    uint32_t minute;
    uint32_t second;
    uint32_t decisecond;
} ShellClock;

/**
 * Creates a Shell Server
 */
int CreateShellServer(uint32_t priority);

#endif /*__SERVER_SHELL_H__*/
