/*
 * kernel.h - provides the constants about the kernel
 */

#define TASK_STACK_SIZE       0x00020000 // 128 KB
#define KERNEL_STACK_SIZE     0x00100000 // 1 MB

#define ADDR_MEMORY_TOP       0x02000000
#define ADDR_KERNEL_STACK_TOP (ADDR_MEMORY_TOP - KERNEL_STACK_SIZE)

/*
 * SYSCALL_INVOKE triggers a SWI syscall.
 */
#define SYSCALL_INVOKE(syscall_code) asm("swi %0" : : "I" (syscall_code))

/*
 * The request code for hardware interrupt
 */
#define HW_INTERRUPT         0x00000000
/*
 * The request codes for syscalls
 */
#define SYSCALL_TASK_CREATE         0x00000001
#define SYSCALL_TASK_YIELD          0x00000002
#define SYSCALL_TASK_EXIT           0x00000003
#define SYSCALL_TASK_GETTID         0x00000004
#define SYSCALL_TASK_GETPTID        0x00000005
#define SYSCALL_TASK_CPUUSAGE       0x00000006
#define SYSCALL_IPC_SEND            0x00000007
#define SYSCALL_IPC_RECV            0x00000008
#define SYSCALL_IPC_REPLY           0x00000009
#define SYSCALL_IRQ_AWAITEVENT      0x0000000A
