#ifndef __SERVER_IO_H__
#define __SERVER_IO_H__

#define COM1_SERVER_NAME "COM1SERVER"
#define COM2_SERVER_NAME "COM2SERVER"
#define COM1 0
#define COM2 1

#include <stdint.h>
#include <utils/queue.h>

typedef enum {
	IO_TRANSMITDOWN,
	IO_TRANSMITUP,
} IOTransmitFlag;

typedef enum {
	IO_CTSINIT,
	IO_CTSDOWN,
	IO_CTSCOMPLETED,
} IOCtsFlag;

typedef enum {
	IO_RECVDOWN,
	IO_RECVUP,
} IORecvFlag;

typedef struct {
	Queue sendqueue;
	Queue recvqueue;
	IOCtsFlag ctsflag;
	IOTransmitFlag transmitflag;
	IORecvFlag recvflag;
} IOStatus;

typedef enum {
	IO_REQUEST_INT_UART,
	IO_REQUEST_PUTC,
	IO_REQUEST_GETC,
} IORequestType;

typedef struct {
	IORequestType type;
   	uint32_t data;
} IORequest;

int CreateIOServer(uint32_t priority, int channel);

#endif
