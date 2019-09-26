/*
 * kernel.h - provides the constants about the kernel
 */

#define TASK_STACK_SIZE       0x00020000 // 128 KB
#define KERNEL_STACK_SIZE     0x00100000 // 1 MB

#define ADDR_MEMORY_TOP       0x02000000
#define ADDR_KERNEL_STACK_TOP (ADDR_MEMORY_TOP - KERNEL_STACK_SIZE)

#define MAX_TASK_NUM        128
#define MAX_TASK_PRIORITY   1024
#define MAX_SYSCALL_ARG_NUM 5

#define SCHEDULER_CALIBRATION 10000

/*
 * The helpers for invoke syscalls
 */
#define SYSCALL_PREPARE(argc) \
    asm("mov r1, %0" : : "I" (argc) : "r1"); \
    asm("mov r2, sp" : : : "r2")

#define SYSCALL_INVOKE(syscall_code) \
    asm("swi %0" : : "I" (syscall_code) : "r1", "r2")

/*
 * The codes for syscalls
 */
#define SYSCALL_IO_GETC      0x00000001
#define SYSCALL_IO_PUTC      0x00000002
#define SYSCALL_TASK_CREATE  0x00000003
#define SYSCALL_TASK_YIELD   0x00000004
#define SYSCALL_TASK_EXIT    0x00000005
#define SYSCALL_TASK_GETTID  0x00000006
#define SYSCALL_TASK_GETPTID 0x00000007
#define SYSCALL_IPC_SEND     0x00000008
#define SYSCALL_IPC_RECV     0x00000009
#define SYSCALL_IPC_REPLY    0x0000000A
