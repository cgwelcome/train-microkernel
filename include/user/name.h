#ifndef __USER_NAME_H__
#define __USER_NAME_H__

/**
 * @defgroup name
 * @ingroup user
 *
 * @{
 */

/**
 * Registers the task id of the caller under the given name.
 * @return 0 success.
 */
int RegisterAs(const char *name);

/**
 * Asks the name server for the task id of the task that is registered under the given name.
 * It blocks if the name hasn't been registered yet.
 * @return the task id of the registered task.
 */
int WhoIs(const char *name);

/** @}*/

#endif
