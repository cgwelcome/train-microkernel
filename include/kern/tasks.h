#ifndef __KERN_TASKS_H__
#define __KERN_TASKS_H__

#include <kern/ipc.h>
#include <utils/queue.h>

#define SCHEDULER_CALIBRATION 1000
#define MAX_TASK_NUM        128
#define MAX_TASK_PRIORITY   4096
#define MAX_SYSCALL_ARG_NUM 5

typedef struct {
    unsigned int r0;
    unsigned int r1;
    unsigned int r2;
    unsigned int r3;
    unsigned int r4;
    unsigned int r5;
    unsigned int r6;
    unsigned int r7;
    unsigned int r8;
    unsigned int r9;
    unsigned int r10;
    unsigned int r11;
    unsigned int r12;
    unsigned int lr;
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
    unsigned long runtime;
    unsigned int priority;
    // Cached Registers
    unsigned int pc;
    Trapframe *tf;
    unsigned int spsr;
    // IPC
    Queue send_queue;
    int *send_tid;
    Message send_msg;
    Message recv_msg;
    Message reply_msg;
} Task;

// swi_handler_init() registers the swi_handler() to the processor.
void swi_handler_init();

// hwi_handler_init() registers the hwi_handler() to the processor.
void hwi_handler_init();

// switch_frame() switches the context from kernel to a user task.
unsigned int switch_frame(unsigned int *pc, Trapframe **tf, unsigned int *sp);

// task_init() initializes the internal variables related to task APIs.
void task_init();

// task_at() returns a pointer to the specified task.
// It return NULL if tid is out of range
Task *task_at(int tid);

// task_create() creates a new task on behalf of the kernel.
// Parameters:
//   ptid: parent tid, which is -1 if the task is started by kernel itself.
//   priority: a positive integer from 1 to 1024.
//   function: a function pointer to the entry of the creating task.
// Return Values:
//   tid: the allocated tid if the task has been created successfully.
//   -1: the priority is invalid.
//   -2: the kernel is out of task descriptors.
int task_create(int ptid, unsigned int priority, void (*function)());

// task_schedule() returns next task tid to activate.
// It returns -1 if there is no task alive.
int task_schedule();

// task_activate() transfers control from kernel to the specified task.
int task_activate(int tid);

// task_kill() terminates the specified task.
void task_kill(int tid);

#endif
