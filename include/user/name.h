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
 * @return  0 success,
 * or -1 invalid name server task id inside wrapper.
 */
int RegisterAs(const char *name);

/**
 * Asks the name server for the task id of the task that is registered under the given name.
 * @return the task id of the registered task,
 * or -1 invalid name server task id inside wrapper.
 */
int WhoIs(const char *name);

/** @}*/

#endif
