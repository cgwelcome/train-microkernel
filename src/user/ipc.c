#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <kernel.h>
#include <user/ipc.h>

int Send(int tid, const char *msg, size_t msglen, char *reply, size_t rplen) {
    register int ret asm("r0");
    asm("ldr r4, %0" : : "m" (rplen) : "r4");
    SYSCALL_INVOKE(SYSCALL_IPC_SEND);
    return ret;
}

int Receive(int *tid, char *msg, size_t msglen) {
    register int ret asm("r0");
    SYSCALL_INVOKE(SYSCALL_IPC_RECV);
    return ret;
}

int Peek(int tid, char *msg, size_t msglen) {
    register int ret asm("r0");
    SYSCALL_INVOKE(SYSCALL_IPC_PEEK);
    return ret;
}

int Reply(int tid, const char *reply, size_t rplen) {
    register int ret asm("r0");
    SYSCALL_INVOKE(SYSCALL_IPC_REPLY);
    return ret;
}
