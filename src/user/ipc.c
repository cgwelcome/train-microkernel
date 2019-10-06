#include <user/ipc.h>
#include <kernel.h>

int Send(int tid, const char *msg, int msglen, char *reply, int rplen) {
    register int ret asm("r0");
    asm("ldr r4, %0" : : "m" (rplen) : "r4");
    SYSCALL_INVOKE(SYSCALL_IPC_SEND);
    return ret;
}

int Receive(int *tid, char *msg, int msglen) {
    register int ret asm("r0");
    SYSCALL_INVOKE(SYSCALL_IPC_RECV);
    return ret;
}

int Reply(int tid, const char *reply, int rplen) {
    register int ret asm("r0");
    SYSCALL_INVOKE(SYSCALL_IPC_REPLY);
    return ret;
}
