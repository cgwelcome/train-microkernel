#ifndef __USER_IPC_H__
#define __USER_IPC_H__

// Send() sends a message to another task and receives a reply.
// Return Values:
//   >-1    the size of the message returned by the replying task.
//    -1    tid is not the task id of an existing task.
//    -2    send-receive-reply transaction could not be completed.
int Send(int tid, const char *msg, int msglen, char *reply, int rplen);

// Receive() blocks until a message is sent to the caller, then
// returns with the message in its message buffer and tid set to
// the task id of the task that sent the message.
//
// Return Values:
//   >-1    the size of the message sent by the sender (stored in tid).
int Receive(int *tid, char *msg, int msglen);

// Reply() sends a reply to a task that previously sent a message.
// Return Values:
//   >-1    the size of the reply message transmitted to the original sender task.
//    -1    tid is not the task id of an existing task.
//    -2    tid is not the task id of a reply-blocked task.
int Reply(int tid, const char *reply, int rplen);

#endif
