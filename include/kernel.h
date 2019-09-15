/*
 * kernel.h - provides the constants about the kernel
 */

#define TASK_STACK_SIZE       0x00100000 // 1MB
#define KERNEL_STACK_SIZE     0x00800000 // 8MB

#define ADDR_MEMORY_TOP       0x02000000
#define ADDR_KERNEL_STACK_TOP (ADDR_MEMORY_TOP - KERNEL_STACK_SIZE)

#define MAX_TASK_NUM 12
#define MAX_TASK_PRIORITY 1024
