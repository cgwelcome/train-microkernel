#include <kernel.h>

int AwaitEvent(int eventid) {
    register int data asm("r0");
    asm("sub sp, sp, #4");
    asm("str %0, [sp]" : : "r" (eventid));
    SYSCALL_PREPARE(1);
    SYSCALL_INVOKE(SYSCALL_IRQ_AWAITEVENT);
    return data;
}
