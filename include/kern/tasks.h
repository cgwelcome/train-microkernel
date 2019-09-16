#ifndef __KERN_TASKS_H__
#define __KERN_TASKS_H__

#include <kernel.h>

typedef enum{Unused, Active, Ready, Zombie} TaskStatus;

typedef struct {
    TaskStatus status;
    unsigned int tid;
    unsigned int ptid;
    unsigned int runtime;
    unsigned int priority;
    void (*code)();
    void *stack;
    int return_value;
} Task;

void task_init();

int task_create(int ptid, unsigned int priority, void (*function)());

// task_schedule() returns next task tid to activate.
// It returns -1 if there is no task alive.
int task_schedule();

int task_activate(int tid);

void task_yield(int tid);

void task_exit(int tid);

#endif // __KERN_TASKS_H__
