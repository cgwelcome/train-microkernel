#ifndef __SERVER_CLOCK_H__
#define __SERVER_CLOCK_H__

#include <stddef.h>
#include <stdint.h>

/**
 * @defgroup clock
 * @ingroup server
 *
 * @{
 */

#define CLOCK_TICK_INTERVAL 10
#define CLOCK_SCHEDULE_JOB_LIMIT    64
#define CLOCK_SCHEDULE_DATA_LIMIT   32

typedef enum {
    CS_NOTIFY,
    CS_TIME,
    CS_DELAY,
    CS_DELAYUNTIL,
    CS_SCHEDULE,
} CSRequestType;

typedef struct {
    CSRequestType type;
    int data;
} CSRequest;

typedef struct {
    uint8_t in_use;
    int     ticks;
    int     target;
    char    data[CLOCK_SCHEDULE_DATA_LIMIT];
    size_t  data_size;
} CSScheduleJob;

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
