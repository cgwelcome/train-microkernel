#ifndef __KERN_IPC_H___
#define __KERN_IPC_H___

#define MAX_MESSAGE_LEN

typedef struct {
    char *array;
    unsigned int len;
} Message;

int ipc_send(int tid, int recvtid, char *msg, int msglen, char *reply, int rplen);
int ipc_receive(int tid, int *sendtid, char *msg, int msglen);
int ipc_reply(int tid, int replytid, char *reply, int rplen);

#endif
