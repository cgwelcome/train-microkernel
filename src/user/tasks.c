#include <user/tasks.h>
#include <kernel.h>

int Create(unsigned int priority, void (*entry)()) {
    register int ret asm("r0");
    asm("sub sp, sp, #8");
    asm("str %0, [sp]" : : "r" (priority));
    asm("str %0, [sp, #4]" : : "r" (entry));
    SYSCALL_PREPARE(2);
    SYSCALL_INVOKE(SYSCALL_TASK_CREATE);
    asm("add sp, #8");
    return ret;
}

void Yield() {
    SYSCALL_PREPARE(0);
    SYSCALL_INVOKE(SYSCALL_TASK_YIELD);
}

void Exit() {
    SYSCALL_PREPARE(0);
    SYSCALL_INVOKE(SYSCALL_TASK_EXIT);
}

int MyTid() {
    register int tid asm("r0");
    SYSCALL_PREPARE(0);
    SYSCALL_INVOKE(SYSCALL_TASK_GETTID);
    return tid;
}

int MyParentTid() {
    register int ptid asm("r0");
    SYSCALL_PREPARE(0);
    SYSCALL_INVOKE(SYSCALL_TASK_GETPTID);
    return ptid;
}
