#ifndef __USER_CLOCK_H__
#define __USER_CLOCK_H__

// Time() returns the number of ticks since the clock server was created and initialized.
// Return Values:
//   >-1    time in ticks since the clock server initialized.
//    -1    tid is not a valid clock server task.
int Time(int tid);

// Delay() returns after the given number of ticks has elapsed.
// Return Values:
//   >-1    success. The current time returned (as in Time())
//    -1    tid is not a valid clock server task.
//    -2    negative delay.
int Delay(int tid, int ticks);

// DelayUntil() returns when the time since clock server initialization is greater or equal than the given number of ticks.
// Return Values:
//   >-1    success. The current time returned (as in Time())
//    -1    tid is not a valid clock server task.
//    -2    negative delay.
int DelayUntil(int tid, int ticks);

#endif
