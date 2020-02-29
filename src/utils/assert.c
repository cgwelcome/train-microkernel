#include <utils/assert.h>
#include <utils/bwio.h>

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

#define ERROR_MSG_MAX_SIZE 256

static __attribute__ ((noinline)) noreturn
void _panic(char *file, int line, char *msg) {
    asm("mrs r12, cpsr");
    register uint32_t CPSR asm("r12");

    if ((CPSR & 0x1F) == PSR_MODE_USR) {
        SYSCALL_INVOKE(SYSCALL_PANIC);
    } else {
        bwprintf(COM2, "\033[2J\033[1;1H");
        bwprintf(COM2, "Panic: %s, at %s:%d.\r\n", msg, file, line);
    }
    while (1);
}

void panic(char *file, int line, char *fmt, ...) {
    va_list va;

    va_start(va, fmt);
    char msg[ERROR_MSG_MAX_SIZE];
    vsnprintf(msg, ERROR_MSG_MAX_SIZE, fmt, va);
    _panic(file, line, msg);
    va_end(va);
}
