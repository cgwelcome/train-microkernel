#include <user/io.h>
#include <kernel.h>

int Getc(int server, int uart) {
    register int ret asm("r0");
    SYSCALL_PREPARE(2);
    SYSCALL_INVOKE(SYSCALL_IO_GETC);
    SYSCALL_CLEANUP;
    return ret;
}

int Putc(int server, int uart, char ch) {
    register int ret asm("r0");
    SYSCALL_PREPARE(3);
    SYSCALL_INVOKE(SYSCALL_IO_PUTC);
    SYSCALL_CLEANUP;
    return ret;
}
