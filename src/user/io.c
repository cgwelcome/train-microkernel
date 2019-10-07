#include <kernel.h>
#include <user/io.h>

int Getc(int server, int uart) {
    register int ret asm("r0");
    SYSCALL_INVOKE(SYSCALL_IO_GETC);
    return ret;
}

int Putc(int server, int uart, char ch) {
    register int ret asm("r0");
    SYSCALL_INVOKE(SYSCALL_IO_PUTC);
    return ret;
}
