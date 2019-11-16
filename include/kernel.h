#ifndef __KERNEL_H__
#define __KERNEL_H__

/**
 * @file provides the constants about the kernel
 */

/**
 * @defgroup hardware
 * @defgroup kernel
 * @defgroup server
 * @defgroup user
 */

/**
 * Common constants
 */
#define COM1 0
#define COM2 1

#define ON  0
#define OFF 1

/**
 * Memory mapping
 */
#define TASK_STACK_SIZE       0x00020000 // 128 KB
#define KERNEL_STACK_SIZE     0x00100000 // 1 MB

#define ADDR_MEMORY_TOP       0x02000000
#define ADDR_KERNEL_STACK_TOP (ADDR_MEMORY_TOP - KERNEL_STACK_SIZE)

/**
 * SYSCALL_INVOKE triggers a SWI syscall.
 */
#define SYSCALL_INVOKE(syscall_code) asm("swi %0" : : "I" (syscall_code))

/**
 * The request code for hardware interrupt
 */
#define HW_INTERRUPT         0x00000000
/**
 * The request codes for syscalls
 */
#define SYSCALL_TASK_CREATE         0x00000001
#define SYSCALL_TASK_YIELD          0x00000002
#define SYSCALL_TASK_EXIT           0x00000003
#define SYSCALL_TASK_GETTID         0x00000004
#define SYSCALL_TASK_GETPTID        0x00000005
#define SYSCALL_TASK_CPUUSAGE       0x00000006
#define SYSCALL_TASK_SHUTDOWN       0x00000007
#define SYSCALL_IPC_SEND            0x00000008
#define SYSCALL_IPC_RECV            0x00000009
#define SYSCALL_IPC_PEEK            0x0000000A
#define SYSCALL_IPC_REPLY           0x0000000B
#define SYSCALL_IRQ_AWAITEVENT      0x0000000C
#define SYSCALL_PANIC               0x0000000D

/**
 * Default task names for important servers
 */
#define SERVER_NAME_CLOCK       "CS"
#define SERVER_NAME_IO          "IO"

#define SERVER_NAME_RPS         "RPS"
#define SERVER_NAME_TRAIN       "TS"

/**
 * Default priorities for important servers
 */
#define PRIORITY_ROOT_TASK           500
#define PRIORITY_IDLE_TASK             1

#define PRIORITY_SERVER_CLOCK       3500
#define PRIORITY_SERVER_IO          3500
#define PRIORITY_SERVER_NAME        4000

#define PRIORITY_SERVER_RPS         2000
#define PRIORITY_SERVER_UI          2000
#define PRIORITY_SERVER_TRAIN       3000

#define PRIORITY_NOTIFIER_CLOCK         2500
#define PRIORITY_NOTIFIER_IO_COM1       2500
#define PRIORITY_NOTIFIER_IO_COM2       2500
#define PRIORITY_NOTIFIER_TRAIN         2500

#endif /*__KERNEL_H__*/
