#include <user/tasks.h>
#include <kernel.h>

extern int current_tid, current_ptid;

int Create(unsigned int priority, void (*entry)()) {
    asm("push {r1, r2}");
        register int ret asm("r0");
        unsigned int args[2] = { priority, (unsigned int) entry };
        asm("mov r1, %0" : : "I" (sizeof(args)));
        asm("mov r2, %0" : : "r" (args));
        asm("swi %0" : : "I" (SYSCALL_TASK_CREATE));
    asm("pop {r1, r2}");
    return ret;
}

void Yield() {
    asm("push {r1, r2}");
        asm("mov r1, #0");
        asm("swi %0" : : "I" (SYSCALL_TASK_YIELD));
    asm("pop {r1, r2}");
}

void Exit() {
    asm("push {r1, r2}");
        asm("mov r1, #0");
        asm("swi %0" : : "I" (SYSCALL_TASK_EXIT));
    asm("pop {r1, r2}");
}

int MyTid() { return current_tid; }

int MyParentTid() { return current_ptid; }
