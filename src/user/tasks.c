#include <user/tasks.h>
#include <kernel.h>

int Create(unsigned int priority, void (*entry)()) {
    register int ret asm("r0");
    SYSCALL_PREPARE(2);
    SYSCALL_INVOKE(SYSCALL_TASK_CREATE);
    SYSCALL_CLEANUP;
    return ret;
}

void Yield() {
    SYSCALL_PREPARE(0);
    SYSCALL_INVOKE(SYSCALL_TASK_YIELD);
    SYSCALL_CLEANUP;
}

void Exit() {
    SYSCALL_PREPARE(0);
    SYSCALL_INVOKE(SYSCALL_TASK_EXIT);
    SYSCALL_CLEANUP;
}

int MyTid() {
    register int tid asm("r0");
    SYSCALL_PREPARE(0);
    SYSCALL_INVOKE(SYSCALL_TASK_GETTID);
    SYSCALL_CLEANUP;
    return tid;
}

int MyParentTid() {
    register int ptid asm("r0");
    SYSCALL_PREPARE(0);
    SYSCALL_INVOKE(SYSCALL_TASK_GETPTID);
    SYSCALL_CLEANUP;
    return ptid;
}
