#include <kernel.h>

int AwaitEvent(int eventid) {
    register int data asm("r0");
    SYSCALL_INVOKE(SYSCALL_IRQ_AWAITEVENT);
    return data;
}
