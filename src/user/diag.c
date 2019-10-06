#include <user/diag.h>
#include <kernel.h>

int MyCpuUsage() {
    register int usage asm("r0");
    SYSCALL_INVOKE(SYSCALL_DIAG_CPUUSAGE);
    return usage;
}
