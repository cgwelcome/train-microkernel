#ifndef __SERVER_SHELL_H__
#define __SERVER_SHELL_H__

#define SENSOR_READ_INTERVAL 100
#define CLOCK_PRECISION 100

#define SHELL_PRIORITY 3000

#include <stdint.h>


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
