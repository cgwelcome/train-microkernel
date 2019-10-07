#ifndef __USER_TASKS_H__
#define __USER_TASKS_H__

#include <stdint.h>

// Create() asks kernel to create a new task.
// Parameters:
//   priority   a positive integer from 1 to 1024.
//   entry      a function pointer to the entry of the creating task.
// Return Values:
//   tid    the allocated tid if the task has been created successfully.
//    -1    the priority is invalid.
//    -2    the kernel is out of task descriptors.
int Create(uint32_t priority, void (*entry)());

// Yield() skips this time slice, force the kernel to pick next task.
void Yield();

// Exit() terminates the current running task.
void Exit();

// MyTid() returns the tid of current task.
int MyTid();

// MyParentTid() returns the tid of parent task.
// Return Values:
//   tid    the tid of the parent task.
//    -1    the current task is started by kernel.
int MyParentTid();

// MyCpuUsage() reports the percentage of CPU runtime for current task.
int MyCpuUsage();

#endif
