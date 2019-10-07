#ifndef __USER_NAME_H__
#define __USER_NAME_H__

// RegisterAs() registers the task id of the caller under the given name.
// Return Values:
//    0     success
//   -1     invalid name server task id inside wrapper.
int RegisterAs(const char *name);

// WhoIs() asks the name server for the task id of the task that is registered under the given name.
//   tid    task id of the registered task.
//    -1    invalid name server task id inside wrapper.
int WhoIs(const char *name);

#endif
