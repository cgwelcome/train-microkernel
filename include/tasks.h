/*
 * tasks.h - provides APIs for the tasks.
 */

#ifndef __TASKS_H__
#define __TASKS_H__

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
} Task;

typedef struct {
    Task tasks[MAX_TASK_NUM];
    unsigned int alive_count;
    unsigned int total_priority;
} TaskGroup;

int Create(unsigned int priority, void (*function)());

unsigned int Schedule();

int Activate(unsigned int tid);

unsigned int MyTid();

unsigned int MyParentTid();

void Yield();

void Exit();

#endif // __TASKS_H__
