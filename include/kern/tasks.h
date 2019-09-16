#ifndef __KERN_TASKS_H__
#define __KERN_TASKS_H__

#include <kernel.h>

typedef enum{Unused, Active, Ready, Zombie} TaskStatus;

typedef struct {
    TaskStatus status;
    int tid;
    int ptid;
    unsigned int runtime;
    unsigned int priority;
    void (*entry)();
    void *stack;
    int return_value;
} Task;

// task_init() initializes the internal variables related to task APIs.
void task_init();

// task_create() creates a new task on behalf of the kernel.
// Parameters:
//   ptid: parent tid, which is -1 if the task is started by kernel itself.
//   priority: a positive integer from 1 to 1024.
//   function: a function pointer to the entry of the creating task.
// Return Values:
//   tid: the allocated tid if the task has been created successfully.
//   -1: the priority is invalid.
//   -2: the kernel is out of task descriptors.
int task_create(int ptid, unsigned int priority, void (*function)());

// task_schedule() returns next task tid to activate.
// It returns -1 if there is no task alive.
int task_schedule();

// task_activate() transfers control from kernel to the specified task.
int task_activate(int tid);

// task_kill() terminates the specified task.
void task_kill(int tid);

#endif // __KERN_TASKS_H__
