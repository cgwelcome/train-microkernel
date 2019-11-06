#ifndef __SERVER_CLOCK_H__
#define __SERVER_CLOCK_H__

#include <stdint.h>

/**
 * @defgroup clock
 * @ingroup server
 *
 * @{
 */

#define CLOCK_TICK_INTERVAL 10

typedef enum {
    CS_NOTIFY,
    CS_TIME,
    CS_DELAY,
    CS_DELAYUNTIL,
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
void CreateClockServer();

/** @}*/

#endif
