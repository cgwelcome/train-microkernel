/*
 * tasks.h
 */

#include<kernel.h>

struct Task {
    int TID;
    void *code;
    void *stack;
};

struct TaskGroup {
    int size;
    int total_priority;
    struct Task tasks[NUM_TASK];
};

int Create(int priority, void (*function)());

int Schedule(struct TaskGroup *tasks);

int Activate(int TID);

int MyTid();

int MyParentTid();

void Yield();

void Exit();
