#ifndef __USER_EVENT_H__
#define __USER_EVENT_H__

/**
 * @defgroup event
 * @ingroup user
 *
 * @{
 */

/** Blocks until the event identified by eventid occurs then returns with volatile data, if any.
 * @return Volatile data, in the form of a positive integer.
 */
int AwaitEvent(int eventid);

/** @{ */

#endif /*__USER_EVENT_H__*/
