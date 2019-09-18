/*
 * kernel.h - provides the constants about the kernel
 */

#define TASK_STACK_SIZE       0x00020000 // 128 KB
#define KERNEL_STACK_SIZE     0x00100000 // 1 MB

#define ADDR_MEMORY_TOP       0x02000000
#define ADDR_KERNEL_STACK_TOP (ADDR_MEMORY_TOP - KERNEL_STACK_SIZE)

#define MAX_TASK_NUM 128
#define MAX_TASK_PRIORITY 1024

/*
 * The codes for syscalls
 */
#define SYSCALL_IO_GETC     0x00000001
#define SYSCALL_IO_PUTC     0x00000002
#define SYSCALL_TASK_CREATE 0x00000003
#define SYSCALL_TASK_YIELD  0x00000004
#define SYSCALL_TASK_EXIT   0x00000005

#define SPSR_MODE_MASK 0x0000001F
#define SPSR_USER_MODE 0x00000010
#define SPSR_SVC_MODE 0x00000013
#define SPSR_THUMB 0x00000020
#define SPSR_FIQ_INTERRUPT 0x00000040
#define SPSR_IRQ_INTERRUPT 0x00000080
