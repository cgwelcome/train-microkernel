#ifndef __USER_IPC_H__
#define __USER_IPC_H__

#include <stddef.h>

/**
 * @defgroup ipc
 * @ingroup user
 *
 * @{
 */

/**
 * Sends a message to another task and receives a reply.
 * @return the size of the message returned by the replying task,
 * or -1 if the target task is no longer alive,
 * or -2 if send-receive-reply transaction could not be completed.
 */
int Send(int tid, const char *msg, size_t msglen, char *reply, size_t rplen);

/**
 * Blocks until a message is sent to the caller, then
 * returns with the message in its message buffer and tid set to
 * the task id of the task that sent the message.
 * @return the size of the message sent by the sender (stored in tid).
 */
int Receive(int *tid, char *msg, size_t msglen);

/**
 * Retrieves the message sent by a reply-blocked task.
 * @return the size of the message sent by the specific task.
 */
int Peek(int tid, char *msg, size_t msglen);

/**
 * Sends a reply to a task that previously sent a message.
 * @return the size of the reply message transmitted to the original sender task.
 */
int Reply(int tid, const char *reply, size_t rplen);

/** @} */

#endif /*__USER_IPC_H__*/
