#include <event.h>
#include <hardware/uart.h>
#include <server/idle.h>
#include <server/io.h>
#include <user/event.h>
#include <user/ipc.h>
#include <user/name.h>
#include <user/tasks.h>
#include <utils/queue.h>

static int io_server_tid, com1_notifier_tid, com2_notifier_tid;
static IOChannel com1_channel;
static IOChannel com2_channel;

static void io_init_uart(int uart) {
    int speed;
    uint8_t bitconfig;
    uint8_t interrupts;
    switch (uart) {
        case COM1:
            speed = 2400;
            bitconfig = WLEN_MASK | STP2_MASK;
            interrupts = UARTEN_MASK;
            break;
        case COM2:
            speed = 115200;
            bitconfig = WLEN_MASK | FEN_MASK;
            interrupts = UARTEN_MASK | RTIEN_MASK;
            break;
        default:
            return;
    }
    uart_set_speed(uart, speed);
    uart_set_bitconfig(uart, bitconfig);
    uart_enable_interrupts(uart, interrupts);
}

static void io_init_channel(int uart) {
    IOChannel *iochannel;
    switch (uart) {
        case COM1:
            iochannel = &com1_channel;
            iochannel->fifo = false;
            iochannel->cts_flag = IO_CTSCOMPLETED;
            break;
        case COM2:
            iochannel = &com2_channel;
            iochannel->fifo = true;
            break;
        default:
            return;
    }
    queue_init(&iochannel->recv_queue);
    queue_init(&iochannel->send_buffer);
    queue_init(&iochannel->recv_buffer);
    iochannel->uart = uart;
    iochannel->rx_flag = IO_RXUP;
    iochannel->tx_flag = IO_TXUP;
}

static IOChannel *iochannel(int uart) {
    switch (uart) {
        case COM1:
            return &com1_channel;
        case COM2:
            return &com2_channel;
        default:
            return NULL;
    }
}

static bool iochannel_haltable(IOChannel *channel) {
    return queue_size(&channel->send_buffer) == 0;
}

static void iochannel_flush_send(IOChannel *channel) {
    int send_count = channel->fifo ? 8 : 1;
    for (int i = 0; i < send_count && queue_size(&channel->send_buffer) > 0; i++) {
        uart_putc(channel->uart, (char)queue_pop(&channel->send_buffer));
    }
}

static void iochannel_flush_recv(IOChannel *channel) {
    int recv_count = channel->fifo ? 8 : 1;
    for (int i = 0; i < recv_count; i++) {
        int c = uart_getc(channel->uart);
        if (c == -1) break;
        if (queue_size(&channel->recv_queue) > 0) {
            int tid = queue_pop(&channel->recv_queue);
            Reply(tid, (char *)&c, sizeof(c));
        } else {
            queue_push(&channel->recv_buffer, c);
        }
    }
}

static void io_server_update_send(IOChannel *channel) {
    if (channel->tx_flag == IO_TXUP && queue_size(&channel->send_buffer) > 0) {
        if (channel->uart == COM1 && channel->cts_flag == IO_CTSCOMPLETED) {
            iochannel_flush_send(channel);
            channel->tx_flag = IO_TXDOWN;
            channel->cts_flag = IO_CTSINIT;
            uart_enableintr(channel->uart, TIEN_MASK | MSIEN_MASK);
        }
        if (channel->uart == COM2) {
            iochannel_flush_send(channel);
            channel->tx_flag = IO_TXDOWN;
            uart_enableintr(channel->uart, TIEN_MASK);
        }
    }
}

static void io_server_update_recv(IOChannel *channel) {
    if (channel->rx_flag == IO_RXDOWN) {
        iochannel_flush_recv(channel);
        channel->rx_flag = IO_RXUP;
        if (channel->uart == COM1) {
            uart_enableintr(channel->uart, RIEN_MASK);
        }
        if (channel->uart == COM2) {
            uart_enableintr(channel->uart, RIEN_MASK | RTIEN_MASK);
        }
    }
}

static void io_server_update_flag(IOChannel *channel, uint32_t flag) {
    if (channel->uart == COM1 && (flag & MIS_MASK)) {
        uart_clearmsintr(channel->uart);
        int uartstatus = uart_readflag(channel->uart);
        if (!(uartstatus & CTS_MASK) && channel->cts_flag == IO_CTSINIT) {
            channel->cts_flag = IO_CTSDOWN;
        }
        else if (uartstatus & CTS_MASK && channel->cts_flag == IO_CTSDOWN) {
            channel->cts_flag = IO_CTSCOMPLETED;
            uart_disableintr(channel->uart, MSIEN_MASK);
        }
    }
    if (channel->uart == COM2 && (flag & RTIS_MASK)) {
        channel->rx_flag = IO_RXDOWN;
        uart_disableintr(channel->uart, RTIEN_MASK);
    }
    if (flag & RIS_MASK) {
        channel->rx_flag = IO_RXDOWN;
        uart_disableintr(channel->uart, RIEN_MASK);
    }
    if (flag & TIS_MASK) {
        channel->tx_flag = IO_TXUP;
        uart_disableintr(channel->uart, TIEN_MASK);
    }
}

static void io_server_update_channel(IOChannel *channel) {
    io_server_update_send(channel);
    io_server_update_recv(channel);
}

static void io_server_putc(IOChannel *channel, int tid, int c) {
    queue_push(&channel->send_buffer, c);
    io_server_update_channel(channel);
    Reply(tid, NULL, 0);
}

static void io_server_putw(IOChannel *channel, int tid, char *str) {
    int c = 0;
    while ((c = *str++)) {
        queue_push(&channel->send_buffer, c);
    }
    io_server_update_channel(channel);
    Reply(tid, NULL, 0);
}

static void io_server_getc(IOChannel *channel, int tid) {
    if (queue_size(&channel->recv_buffer) != 0) {
        int c = queue_pop(&channel->recv_buffer);
        Reply(tid, (char *)&c, sizeof(c));
    } else {
        queue_push(&channel->recv_queue, tid);
        uart_enableintr(channel->uart, RIEN_MASK);
    }
}

void io_server_task() {
    int tid; IORequest request; IOChannel *channel;
    bool is_halting = false; int halting_error = -1;
    Queue halting_queue; queue_init(&halting_queue);

    RegisterAs(IO_SERVER_NAME);
    for (;;) {
        Receive(&tid, (char *)&request, sizeof(request));
        if (is_halting) {
            if (request.type != IO_REQUEST_INT_UART && request.type != IO_REQUEST_SHUTDOWN) {
                Reply(tid, (char *) &halting_error, sizeof(halting_error));
                continue;
            }
        }
        switch (request.type) {
            case IO_REQUEST_INT_UART:
                channel = iochannel(request.uart);
                io_server_update_flag(channel, request.data);
                io_server_update_channel(channel);
                Reply(tid, NULL, 0);
                break;
            case IO_REQUEST_PUTC:
                channel = iochannel(request.uart);
                io_server_putc(channel, tid, (int)request.data);
                break;
            case IO_REQUEST_PUTW:
                channel = iochannel(request.uart);
                io_server_putw(channel, tid, (char *)request.data);
                break;
            case IO_REQUEST_GETC:
                channel = iochannel(request.uart);
                io_server_getc(channel, tid);
                break;
            case IO_REQUEST_SHUTDOWN:
                if (iochannel_haltable(&com1_channel) && iochannel_haltable(&com2_channel)) {
                    is_halting = true;
                    Reply(tid, NULL, 0);
                } else {
                    is_halting = true;
                    queue_push(&halting_queue, tid);
                }
                break;
            default:
                break;
        }
        if (is_halting) {
            if (iochannel_haltable(&com1_channel) && iochannel_haltable(&com2_channel)) {
                while (queue_size(&halting_queue)) {
                    int tid = queue_pop(&halting_queue);
                    Reply(tid, NULL, 0);
                }
                Exit();
            }
        }
    }
}

static void io_com1_notifier_task() {
    int event = INT_UART1;
    IORequest request = {
        .type = IO_REQUEST_INT_UART,
        .uart = COM1
    };
    for (;;) {
        int uartintr = AwaitEvent(event);
        request.data = (uint32_t) uartintr;
        Send(io_server_tid, (char *)&request, sizeof(request), NULL, 0);
    }
}

static void io_com2_notifier_task() {
    int event = INT_UART2;
    IORequest request = {
        .type = IO_REQUEST_INT_UART,
        .uart = COM2
    };
    for (;;) {
        int uartintr = AwaitEvent(event);
        request.data = (uint32_t) uartintr;
        Send(io_server_tid, (char *)&request, sizeof(request), NULL, 0);
    }
}

void InitIOServer() {
    io_server_tid = -1;
    com1_notifier_tid = -1;
    com2_notifier_tid = -1;

    io_init_uart(COM1);
    io_init_uart(COM2);
    io_init_channel(COM1);
    io_init_channel(COM2);
}

int CreateIOServer(uint32_t server_priority, uint32_t com1_priority, uint32_t com2_priority) {
    if (io_server_tid < 0) {
        io_server_tid = Create(server_priority, &io_server_task);
    }
    if (com1_notifier_tid < 0) {
        com1_notifier_tid = Create(com1_priority, &io_com1_notifier_task);
    }
    if (com2_notifier_tid < 0) {
        com2_notifier_tid = Create(com2_priority, &io_com2_notifier_task);
    }
    return io_server_tid;
}

void ShutdownIOServer() {
    IORequest request = {
        .type = IO_REQUEST_SHUTDOWN
    };
    Send(io_server_tid, (char *)&request, sizeof(request), NULL, 0);
}
