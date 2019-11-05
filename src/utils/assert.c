#include <utils/assert.h>

void panic(char *expr, char *file, int line) {
    asm("mrs r12, cpsr");
    register uint32_t CPSR asm("r12");

    if (!(CPSR & PSR_MODE_SVC)) {
        SYSCALL_INVOKE(SYSCALL_PANIC);
    } else {
        bwprintf(COM2, "\033[2J\033[1;1H");
        bwprintf(COM2, "Panic: %s, at %s:%d.\r\n", expr, file, line);
        while (1);
    }
}
