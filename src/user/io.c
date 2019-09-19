#include <user/io.h>
#include <kernel.h>

int Getc(int server, int uart) {
    unsigned int args[2] = { server, uart };
    int ret;
    asm("mov r1, %0" : : "I" (sizeof(args)));
    asm("mov r2, %0" : : "r" (args));
    asm("swi %0" : : "I" (SYSCALL_IO_GETC));
    asm("mov %0, r0" : "=r" (ret) : );
    return ret;
}

int Putc(int server, int uart, char ch) {
    unsigned int args[3] = { server, uart, ch };
    int ret;
    asm("mov r1, %0" : : "I" (sizeof(args)));
    asm("mov r2, %0" : : "r" (args));
    asm("swi %0" : : "I" (SYSCALL_IO_PUTC));
    asm("mov %0, r0" : "=r" (ret) : );
    return ret;
}
