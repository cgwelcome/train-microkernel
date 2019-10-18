#ifndef __KERN_IPC_H___
#define __KERN_IPC_H___

#include <stddef.h>
#include <stdint.h>

typedef struct {
    char *array;
    size_t len;
} Message;

/**
 * Sends message(msg) to recvtid, and waits for a reply message to be written to reply
 */
void ipc_send(int tid, int recvtid, char *msg, size_t msglen, char *reply, size_t rplen);

/**
 * Copies a sender's message to msg and the sender's tid to sendtid.
 */
void ipc_receive(int tid, int *sendtid, char *msg, size_t msglen);

/**
 * Copies the reply message from tid Task to replytid Task.
 */
void ipc_reply(int tid, int replytid, char *reply, size_t rplen);

/**
 * Clears the send queue by unblock the tasks, and return an error code.
 * for incomplete transaction
 */
void ipc_cleanup(int tid);

#endif
