#include <user/ipc.h>
#include <kernel.h>

int Send(int tid, const char *msg, int msglen, char *reply, int rplen) {
    register int ret asm("r0");
    asm("push {r1, r2}");
        unsigned int args[5] = { tid, (unsigned int) msg, msglen, (unsigned int) reply, rplen };
        asm("mov r1, %0" : : "I" (sizeof(args)));
        asm("mov r2, %0" : : "r" (args));
        asm("swi %0" : : "I" (SYSCALL_IPC_SEND));
    asm("pop {r1, r2}");
    return ret;
}

int Receive(int *tid, char *msg, int msglen) {
    register int ret asm("r0");
    asm("push {r1, r2}");
        unsigned int args[3] = { (unsigned int) tid, (unsigned int) msg, msglen };
        asm("mov r1, %0" : : "I" (sizeof(args)));
        asm("mov r2, %0" : : "r" (args));
        asm("swi %0" : : "I" (SYSCALL_IPC_RECV));
    asm("pop {r1, r2}");
    return ret;
}

int Reply(int tid, const char *reply, int rplen) {
    register int ret asm("r0");
    asm("push {r1, r2}");
        unsigned int args[5] = { tid, (unsigned int) reply, rplen };
        asm("mov r1, %0" : : "I" (sizeof(args)));
        asm("mov r2, %0" : : "r" (args));
        asm("swi %0" : : "I" (SYSCALL_IPC_REPLY));
    asm("pop {r1, r2}");
    return ret;
}
