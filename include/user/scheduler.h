#ifndef __USER_SCHEDULER_H__
#define __USER_SCHEDULER_H__

/**
 * @defgroup scheduler
 * @ingroup user
 *
 * @{
 */

/**
 * Send a IPC request to target tid after ticks espaced.
 */
void Schedule(int tid, int ticks, int target, char *data, size_t data_size);

/** @} */

#endif /*__USER_SCHEDULER_H__*/
