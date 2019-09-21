#include <user/ipc.h>
#include <kernel.h>

int Send(int tid, const char *msg, int msglen, char *reply, int rplen) {
    register int ret asm("r0");
    SYSCALL_PREPARE(5);
    SYSCALL_INVOKE(SYSCALL_IPC_SEND);
    SYSCALL_CLEANUP;
    return ret;
}

int Receive(int *tid, char *msg, int msglen) {
    register int ret asm("r0");
    SYSCALL_PREPARE(3);
    SYSCALL_INVOKE(SYSCALL_IPC_RECV);
    SYSCALL_CLEANUP;
    return ret;
}

int Reply(int tid, const char *reply, int rplen) {
    register int ret asm("r0");
    SYSCALL_PREPARE(3);
    SYSCALL_INVOKE(SYSCALL_IPC_REPLY);
    SYSCALL_CLEANUP;
    return ret;
}
