#ifndef __SERVER_IDLE_H__
#define __SERVER_IDLE_H__

#include <stdint.h>

/**
 * Creates an idle task for diagnostic purpose.
 */
int CreateIdleTask(uint32_t priority);

#endif
