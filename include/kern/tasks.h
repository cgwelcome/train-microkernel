#ifndef __KERN_TASKS_H__
#define __KERN_TASKS_H__

#include <stdint.h>
#include <kern/ipc.h>
#include <utils/queue.h>

/**
 * @defgroup tasks
 * @ingroup kernel
 *
 * @{
 */

#define SCHEDULER_CALIBRATION 1000
#define MAX_TASK_NUM        128
#define MAX_TASK_PRIORITY   4096
#define MAX_SYSCALL_ARG_NUM 5

typedef struct {
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r4;
    uint32_t r5;
    uint32_t r6;
    uint32_t r7;
    uint32_t r8;
    uint32_t r9;
    uint32_t r10;
    uint32_t r11;
    uint32_t r12;
    uint32_t lr;
} Trapframe;

typedef enum {
    UNUSED,
    ACTIVE,
    READY,
    ZOMBIE,
    RECVBLOCKED,
    SENDBLOCKED,
    REPLYBLOCKED,
    EVENTBLOCKED,
} TaskStatus;

typedef struct {
    // Task Properties
    TaskStatus status;
    int tid;
    int ptid;
    uint64_t runtime;
    uint32_t priority;
    // Cached Registers
    uint32_t pc;
    Trapframe *tf;
    uint32_t spsr;
    // IPC
    Queue send_queue;
    int *send_tid;
    Message send_msg;
    Message recv_msg;
    Message reply_msg;
} Task;

/**
 * Registers the swi_handler() to the processor.
 */
void swi_handler_init();

/**
 * Registers the hwi_handler() to the processor.
 */
void hwi_handler_init();

/**
 * Switches the context from kernel to a user task.
 */
uint32_t switch_frame(uint32_t *pc, Trapframe **tf, uint32_t *sp);

/**
 * Initializes the internal variables related to task APIs.
 */
void task_init();

/**
 * Returns a pointer to the specified task.
 * It return NULL if tid is out of range
 */
Task *task_at(int tid);

/**
 * Creates a new task on behalf of the kernel.
 * Parameters:
 *  ptid: parent tid, which is -1 if the task is started by kernel itself.
 *  priority: a positive integer from 1 to 1024.
 *  function: a function pointer to the entry of the creating task.
 *  arg: an argument passed to the entry of the creating task.
 * Return Values:
 *  tid: the allocated tid if the task has been created successfully.
 *  -1: the priority is invalid.
 *  -2: the kernel is out of task descriptors.
 */
int task_create(int ptid, uint32_t priority, void (*function)(), uint32_t arg);

/**
 * Returns next task tid to activate.
 * It returns -1 if there is no task alive.
 */
int task_schedule();

/**
 * Transfers control from kernel to the specified task.
 */
uint32_t task_activate(int tid);

/**
 * Terminates the specified task.
 */
void task_kill(int tid);

void task_shutdown();

/** @}*/

#endif
