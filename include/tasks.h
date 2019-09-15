/*
 * tasks.h - provides APIs for the tasks.
 */

#ifndef __TASKS_H__
#define __TASKS_H__

typedef struct {
    int TID;
    int PTID;
    void *code;
    void *stack;
} Task;

typedef struct {
    int size;
    Task tasks[MAX_TASK_NUM];
    int total_priority;
} TaskGroup;

int Create(int priority, void (*function)());

int Schedule();

int Activate(int TID);

int MyTid();

int MyParentTid();

void Yield();

void Exit();

#endif // __TASKS_H__
