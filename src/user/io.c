#include <user/io.h>
#include <kernel.h>

int Getc(int server, int uart) {
    int ret;
    asm("mov r1, %0" : : "r" (server));
    asm("mov r2, %0" : : "r" (uart));
    asm("swi %0" : : "I" (SYSCALL_IO_GETC));
    asm("mov %0, r0" : "=r" (ret) : );
    return ret;
}

int Putc(int server, int uart, char ch) {
    int ret;
    asm("mov r1, %0" : : "r" (server));
    asm("mov r2, %0" : : "r" (uart));
    asm("mov r3, %0" : : "r" (ch));
    asm("swi %0" : : "I" (SYSCALL_IO_PUTC));
    asm("mov %0, r0" : "=r" (ret) : );
    return ret;
}
