#ifndef __KERN_TASKS_H__
#define __KERN_TASKS_H__

#include <kernel.h>
#include <kern/ipc.h>
#include <kern/switchframe.h>
#include <utils/queue.h>

#define SCHEDULER_CALIBRATION 1000
#define MAX_TASK_NUM        128
#define MAX_TASK_PRIORITY   4096
#define MAX_SYSCALL_ARG_NUM 5

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
    unsigned int runtime;
    unsigned int priority;
    // Cached Registers
    unsigned int pc;
    Trapframe *tf;
    unsigned int spsr;
    // Syscall related fields
    int syscall_args[MAX_SYSCALL_ARG_NUM];
    int return_value;
    // IPC
    Message send_msg;
    Message recv_msg;
    int *send_tid;
    Message reply_msg;
    Queue send_queue;
} Task;
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

// task_cputime() gets the cpu usage of the specified task
int task_cputime(int tid);

// task_setstarttime() sets the start time of the kernel used to calculat
void task_setstarttime(int time);

// task_cpuusage() gets the the fraction of time the task has ran
int task_cpuusage(int tid);

#endif // __KERN_TASKS_H__
