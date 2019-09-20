#ifndef __USER_IPC_H__
#define __USER_IPC_H__

int Send(int tid, const char *msg, int msglen, char *reply, int rplen);

int Receive(int *tid, char *msg, int msglen);

int Reply(int tid, const char *reply, int rplen);

#endif // __USER_IPC_H__
