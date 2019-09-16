#ifndef __USER_TASKS_H__
#define __USER_TASKS_H__

int Create(unsigned int priority, void (*function)());

unsigned int MyTid();

unsigned int MyParentTid();

void Yield();

void Exit();

#endif // __USER_TASKS_H__
