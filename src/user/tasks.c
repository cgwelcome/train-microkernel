#include <user/tasks.h>
#include <kernel.h>

int Create(unsigned int priority, void (*function)()) {
    int ret;
    asm("mov r1, %0" : : "r" (priority));
    asm("mov r2, %0" : : "r" (function));
    asm("swi %0" : : "I" (SYSCALL_TASK_CREATE));
    asm("mov %0, r0" : "=r" (ret) : );
    return ret;
}

int MyTid() { return 0; }

int MyParentTid() { return 0; }

void Yield() {
    asm("swi %0" : : "I" (SYSCALL_TASK_YIELD));
}

void Exit() {
    asm("swi %0" : : "I" (SYSCALL_TASK_EXIT));
}
