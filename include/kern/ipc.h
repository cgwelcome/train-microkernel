#ifndef __KERN_IPC_H___
#define __KERN_IPC_H___

typedef struct {
    char *array;
    unsigned int len;
} Message;

// ipc_send() sends message(msg) to recvtid, and waits for a reply message to be written to reply
void ipc_send(int tid, int recvtid, char *msg, int msglen, char *reply, int rplen);

// ipc_receive() copies a sender's message to msg and the sender's tid to sendtid.
void ipc_receive(int tid, int *sendtid, char *msg, int msglen);

// ipc_reply() copies the reply message from tid Task to replytid Task.
void ipc_reply(int tid, int replytid, char *reply, int rplen);

// ipc_cleanup() clears the send queue by unblock the tasks, and return an error code.
// for incomplete transaction
void ipc_cleanup(int tid);

#endif
