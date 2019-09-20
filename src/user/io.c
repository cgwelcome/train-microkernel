#include <user/io.h>
#include <kernel.h>

int Getc(int server, int uart) {
    asm("push {r1, r2}");
        register int ret asm("r0");
        unsigned int args[2] = { server, uart };
        asm("mov r1, %0" : : "I" (sizeof(args)));
        asm("mov r2, %0" : : "r" (args));
        asm("swi %0" : : "I" (SYSCALL_IO_GETC));
    asm("pop {r1, r2}");
    return ret;
}

int Putc(int server, int uart, char ch) {
    asm("push {r1, r2}");
        register int ret asm("r0");
        unsigned int args[3] = { server, uart, ch };
        asm("mov r1, %0" : : "I" (sizeof(args)));
        asm("mov r2, %0" : : "r" (args));
        asm("swi %0" : : "I" (SYSCALL_IO_PUTC));
    asm("pop {r1, r2}");
    return ret;
}
