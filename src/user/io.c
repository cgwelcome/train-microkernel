#include <user/io.h>
#include <kernel.h>

int Getc(int server, int uart) {
    register int ret asm("r0");
    asm("sub sp, sp, #8");
    asm("str %0, [sp]" : : "r" (server));
    asm("str %0, [sp, #4]" : : "r" (uart));
    SYSCALL_PREPARE(2);
    SYSCALL_INVOKE(SYSCALL_IO_GETC);
    asm("add sp, sp, #8");
    return ret;
}

int Putc(int server, int uart, char ch) {
    register int ret asm("r0");
    asm("sub sp, sp, #12");
    asm("str %0, [sp]" : : "r" (server));
    asm("str %0, [sp, #4]" : : "r" (uart));
    asm("str %0, [sp, #8]" : : "r" (ch));
    SYSCALL_PREPARE(3);
    SYSCALL_INVOKE(SYSCALL_IO_PUTC);
    asm("add sp, sp, #12");
    return ret;
}
