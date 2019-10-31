#ifndef __SERVER_CLOCK_H__
#define __SERVER_CLOCK_H__

#include <stdint.h>

/**
 * @defgroup clock
 * @ingroup server
 *
 * @{
 */

#define CLOCK_NOTIFY_INTERVAL 10
#define CLOCK_SERVER_NAME "CS"

typedef enum {
    CS_TICKUPDATE,
    CS_DELAY,
    CS_DELAYUNTIL,
    CS_TIME,
} CSRequestType;

typedef struct {
    CSRequestType type;
    int data;
} CSRequest;

/**
 * Initialize some global counters for the Clock Server.
 */
void InitClockServer();

/**
 * Creates a global Clock Server
 */
int CreateClockServer(uint32_t priority);

/** @}*/

#endif
