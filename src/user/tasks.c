#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <kernel.h>
#include <user/tasks.h>

int Create(uint32_t priority, void (*entry)()) {
    register int ret asm("r0");
    SYSCALL_INVOKE(SYSCALL_TASK_CREATE);
    return ret;
}

void Yield() {
    SYSCALL_INVOKE(SYSCALL_TASK_YIELD);
}

void Exit() {
    SYSCALL_INVOKE(SYSCALL_TASK_EXIT);
}

int MyTid() {
    register int tid asm("r0");
    SYSCALL_INVOKE(SYSCALL_TASK_GETTID);
    return tid;
}

int MyParentTid() {
    register int ptid asm("r0");
    SYSCALL_INVOKE(SYSCALL_TASK_GETPTID);
    return ptid;
}

int MyCpuUsage() {
    register int usage asm("r0");
    SYSCALL_INVOKE(SYSCALL_TASK_CPUUSAGE);
    return usage;
}

int Shutdown() {
    register int status asm("r0");
    SYSCALL_INVOKE(SYSCALL_TASK_SHUTDOWN);
    return status;
}
