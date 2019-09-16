#ifndef __USER_TASKS_H__
#define __USER_TASKS_H__

int Create(unsigned int priority, void (*function)());

int MyTid();

int MyParentTid();

void Yield();

void Exit();

#endif // __USER_TASKS_H__
