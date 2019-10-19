#ifndef __USER_EVENT_H__
#define __USER_EVENT_H__

// AwaitEvent() blocks until the event identified by eventid occurs then returns with volatile data, if any.
// Return Values:
//   >-1    volatile data, in the form of a positive integer.
//    -1    invalid event.
//    -2    corrupted volatile data.
int AwaitEvent(int eventid);

#endif
