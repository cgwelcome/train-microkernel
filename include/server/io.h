#ifndef __SERVER_IO_H__
#define __SERVER_IO_H__

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <kernel.h>
#include <utils/queue.h>

/**
 * @defgroup io
 * @ingroup server
 *
 * @{
 */

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
    Queue send_queue;
    Queue recv_queue;
    Queue send_buffer;
    Queue recv_buffer;
    IOCTSFlag cts_flag;
    IOTXFlag tx_flag;
    IORXFlag rx_flag;
} IOChannel;

typedef enum {
    IO_REQUEST_INT_UART,
    IO_REQUEST_PUT,
    IO_REQUEST_GET,
    IO_REQUEST_SHUTDOWN,
} IORequestType;

typedef struct {
    IORequestType type;
    int uart;
    uint32_t data;
    size_t size;
} IORequest;

void InitIOServer();

void CreateIOServer();

void ShutdownIOServer();

/** @}*/

#endif /*__SERVER_IO_H__*/
