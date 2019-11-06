#ifndef __SERVER_SCHEDULER_H__
#define __SERVER_SCHEDULER_H__

#include <stdint.h>
#include <stddef.h>

/**
 * @defgroup scheduler
 * @ingroup server
 *
 * @{
 */

#define SCHEDULER_TICK_INTERVAL 10
#define SCHEDULER_JOB_BUFFER_SIZE    64
#define SCHEDULER_CALLBACK_DATA_SIZE 32

typedef enum {
    SS_NOTIFY,
    SS_SCHEDULE,
    SS_DONE,
} SSRequestType;

typedef struct {
    SSRequestType type;
    int ticks;
    int target;
    char data[SCHEDULER_CALLBACK_DATA_SIZE];
    size_t data_size;
} SSRequest;

/**
 * Initialize some global counters for the Scheduler Server.
 */
void InitSchedulerServer();

/**
 * Creates a global Scheduler Server
 */
void CreateSchedulerServer();

/** @}*/

#endif
