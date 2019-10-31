#ifndef __USER_CLOCK_H__
#define __USER_CLOCK_H__

/**
 * @defgroup clock
 * @ingroup user
 *
 * @{
 */

/**
 * Returns the number of ticks since the clock server
 * was created and initialized.
 * @return the Time in ticks since the clock server initialized, or -1 if
 * tid is not a valid clock server task.
 */
int Time(int tid);

/**
 * Returns after the given number of ticks has elapsed.
 * @return the current time returned (as in Time()),
 * or -1 tid is not a valid clock server task,
 * or -2 negative delay.
 */
int Delay(int tid, int ticks);

/**
 * Returns when the time since clock server initialization is
 * greater or equal than the given number of ticks.
 * @return the current time returned (as in Time()),
 *  or -1 tid is not a valid clock server task.
 *  or -2 negative delay.
 */
int DelayUntil(int tid, int ticks);

/** @} */

#endif /*__USER_CLOCK_H__*/
