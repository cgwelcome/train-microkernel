#ifndef __SERVER_IO_H__
#define __SERVER_IO_H__

#define IO_SERVER_NAME "IO"
#define COM1 0
#define COM2 1

#include <stdbool.h>
#include <stdint.h>
#include <utils/queue.h>

typedef enum {
    IO_CTSINIT,
    IO_CTSDOWN,
    IO_CTSCOMPLETED,
} IOCTSFlag;

typedef enum {
    IO_TXDOWN,
    IO_TXUP,
} IOTXFlag;

typedef enum {
    IO_RXDOWN,
    IO_RXUP,
} IORXFlag;

typedef struct {
    int uart;
    bool fifo;
    Queue recv_queue;
    Queue send_buffer;
    Queue recv_buffer;
    IOCTSFlag cts_flag;
    IOTXFlag tx_flag;
    IORXFlag rx_flag;
} IOChannel;

typedef enum {
    IO_REQUEST_INT_UART,
    IO_REQUEST_PUTC,
    IO_REQUEST_PUTW,
    IO_REQUEST_GETC,
    IO_REQUEST_SHUTDOWN,
} IORequestType;

typedef struct {
    IORequestType type;
    int uart;
    uint32_t data;
} IORequest;

void InitIOServer();

int CreateIOServer(uint32_t server_priority, uint32_t com1_priority, uint32_t com2_priority);

void ShutdownIOServer();

#endif
