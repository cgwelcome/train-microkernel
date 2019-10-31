#ifndef __USER_TASKS_H__
#define __USER_TASKS_H__

#include <stdint.h>

/**
 * @defgroup task
 * @ingroup user
 *
 * @{
 */

/**
 * Asks kernel to create a new task.
 * Parameters:
 * @param[in] priority A positive integer from 1 to 1024.
 * @param[in] entry    A function pointer to the entry of the creating task.
 * @return the allocated tid if the task has been created successfully,
 * or -1 the priority is invalid,
 * or -2 the kernel is out of task descriptors.
 */
int Create(uint32_t priority, void (*entry)());

/**
 * Skips this time slice, force the kernel to pick next task.
 */
void Yield();

/**
 * Terminates the current running task.
 */
void Exit();

/**
 * Returns the tid of current task.
 */
int MyTid();

/**
 * Returns the tid of parent task.
 * @return the tid of the parent task,
 * or -1 the current task is started by kernel.
 */
int MyParentTid();

/**
 * Reports the percentage of CPU runtime for current task.
 */
int MyCpuUsage();

/**
 * Kills all tasks
 */
int Shutdown();

/** @}*/

#endif /*__USER_TASKS_H__*/
