#ifndef __SERVER_SHELL_H__
#define __SERVER_SHELL_H__

#define SENSOR_READ_INTERVAL 100

#include <stdint.h>

/**
 * Creates a Shell Server
 */
void CreateShellServer(uint32_t priority);

#endif /*__SERVER_SHELL_H__*/
