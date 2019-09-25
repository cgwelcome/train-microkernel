#include <user/ipc.h>
#include <kernel.h>

int Send(int tid, const char *msg, int msglen, char *reply, int rplen) {
    register int ret asm("r0");
    asm("sub sp, sp, #20");
    asm("str %0, [sp]" : : "r" (tid));
    asm("str %0, [sp, #4]" : : "r" (msg));
    asm("str %0, [sp, #8]" : : "r" (msglen));
    asm("str %0, [sp, #12]" : : "r" (reply));
    asm("str %0, [sp, #16]" : : "r" (rplen));
    SYSCALL_PREPARE(5);
    SYSCALL_INVOKE(SYSCALL_IPC_SEND);
    asm("add sp, sp, #20");
    return ret;
}

int Receive(int *tid, char *msg, int msglen) {
    register int ret asm("r0");
    asm("sub sp, sp, #12");
    asm("str %0, [sp]" : : "r" (tid));
    asm("str %0, [sp, #4]" : : "r" (msg));
    asm("str %0, [sp, #8]" : : "r" (msglen));
    SYSCALL_PREPARE(3);
    SYSCALL_INVOKE(SYSCALL_IPC_RECV);
    asm("add sp, sp, #12");
    return ret;
}

int Reply(int tid, const char *reply, int rplen) {
    register int ret asm("r0");
    asm("sub sp, sp, #12");
    asm("str %0, [sp]" : : "r" (tid));
    asm("str %0, [sp, #4]" : : "r" (reply));
    asm("str %0, [sp, #8]" : : "r" (rplen));
    SYSCALL_PREPARE(3);
    SYSCALL_INVOKE(SYSCALL_IPC_REPLY);
    asm("add sp, sp, #12");
    return ret;
}
