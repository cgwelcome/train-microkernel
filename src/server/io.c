#include <event.h>
#include <hardware/uart.h>
#include <hardware/timer.h>
#include <server/idle.h>
#include <server/io.h>
#include <user/event.h>
#include <user/ipc.h>
#include <user/name.h>
#include <user/tasks.h>
#include <utils/kassert.h>
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
    queue_init(&iochannel->send_queue);
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

static size_t iochannel_buffer_put(IOChannel *channel, char *buffer, size_t size) {
    kassert(size <= QUEUE_SIZE);
    if (queue_size(&channel->send_buffer) + size <= QUEUE_SIZE) {
        for (size_t i = 0; i < size; i++) {
            queue_push(&channel->send_buffer, (int) buffer[i]);
        }
        return size;
    }
    return 0;
}

static size_t iochannel_buffer_get(IOChannel *channel, char *buffer, size_t size) {
    kassert(size <= QUEUE_SIZE);
    if (queue_size(&channel->recv_buffer) >= size) {
        for (size_t i = 0; i < size; i++) {
            buffer[i] = (char) queue_pop(&channel->recv_buffer);
        }
        return size;
    }
    return 0;
}

static void iochannel_flush_send_buffer(IOChannel *channel) {
    int send_count = channel->fifo ? 8 : 1;
    for (int i = 0; i < send_count && queue_size(&channel->send_buffer) > 0; i++) {
        uart_putc(channel->uart, (char)queue_pop(&channel->send_buffer));
    }
}

static void iochannel_flush_send_queue(IOChannel *channel) {
    IORequest request;
    if (queue_size(&channel->send_queue) > 0 && queue_size(&channel->send_buffer) < QUEUE_SIZE / 2) {
        int tid = queue_peek(&channel->send_queue);
        Peek(tid, (char *)&request, sizeof(request));

        size_t len = iochannel_buffer_put(channel, (char *)request.data, request.size);
        if (len > 0) {
            queue_pop(&channel->send_queue);
            Reply(tid, NULL, 0);
        }
    }
}

static void iochannel_flush_recv_buffer(IOChannel *channel) {
    int recv_count = channel->fifo ? 8 : 1;
    for (int i = 0; i < recv_count && queue_size(&channel->recv_buffer) < QUEUE_SIZE; i++) {
        int c = uart_getc(channel->uart);
        if (c == -1) break;
        queue_push(&channel->recv_buffer, c);
    }
}

static void iochannel_flush_recv_queue(IOChannel *channel) {
    IORequest request;
    while (queue_size(&channel->recv_queue) > 0 && queue_size(&channel->recv_buffer) != 0) {
        int tid = queue_peek(&channel->recv_queue);
        Peek(tid, (char *)&request, sizeof(request));

        size_t len = iochannel_buffer_get(channel, (char *)request.data, request.size);
        if (len > 0) {
            queue_pop(&channel->recv_queue);
            Reply(tid, NULL, 0);
        }
    }
}

static void io_server_update_send(IOChannel *channel) {
    if (channel->tx_flag == IO_TXUP && queue_size(&channel->send_buffer) > 0) {
        if (channel->uart == COM1 && channel->cts_flag == IO_CTSCOMPLETED) {
            iochannel_flush_send_buffer(channel);
            iochannel_flush_send_queue(channel);
            channel->tx_flag = IO_TXDOWN;
            channel->cts_flag = IO_CTSINIT;
            uart_enable_interrupts(channel->uart, TIEN_MASK | MSIEN_MASK);
        }
        if (channel->uart == COM2) {
            iochannel_flush_send_buffer(channel);
            iochannel_flush_send_queue(channel);
            channel->tx_flag = IO_TXDOWN;
            uart_enable_interrupts(channel->uart, TIEN_MASK);
        }
    }
}

static void io_server_update_recv(IOChannel *channel) {
    if (channel->rx_flag == IO_RXDOWN) {
        iochannel_flush_recv_buffer(channel);
        iochannel_flush_recv_queue(channel);
        channel->rx_flag = IO_RXUP;
        if (channel->uart == COM1) {
            uart_enable_interrupts(channel->uart, RIEN_MASK);
        }
        if (channel->uart == COM2) {
            uart_enable_interrupts(channel->uart, RIEN_MASK | RTIEN_MASK);
        }
    }
}

static void io_server_disable_interrupts(int uart, uint32_t interrupts) {
    if (interrupts &  MIS_MASK) uart_disable_interrupts(uart, MSIEN_MASK);
    if (interrupts & RTIS_MASK) uart_disable_interrupts(uart, RTIEN_MASK);
    if (interrupts &  RIS_MASK) uart_disable_interrupts(uart,  RIEN_MASK);
    if (interrupts &  TIS_MASK) uart_disable_interrupts(uart,  TIEN_MASK);
}

static void io_server_update_flag(IOChannel *channel, uint32_t flag) {
    if (channel->uart == COM1 && (flag & MIS_MASK)) {
        int uartstatus = uart_read_flags(channel->uart);
        if (!(uartstatus & CTS_MASK) && channel->cts_flag == IO_CTSINIT) {
            channel->cts_flag = IO_CTSDOWN;
        }
        else if (uartstatus & CTS_MASK && channel->cts_flag == IO_CTSDOWN) {
            channel->cts_flag = IO_CTSCOMPLETED;
        }
    }
    if (channel->uart == COM2 && (flag & RTIS_MASK)) {
        channel->rx_flag = IO_RXDOWN;
    }
    if (flag & RIS_MASK) {
        channel->rx_flag = IO_RXDOWN;
    }
    if (flag & TIS_MASK) {
        channel->tx_flag = IO_TXUP;
    }
}

static void io_server_update_channel(IOChannel *channel) {
    io_server_update_send(channel);
    io_server_update_recv(channel);
}

void io_server_task() {
    int tid; IORequest request; IOChannel *channel; size_t len;
    bool is_halting = false; uint64_t halting_start = 0;
    Queue halting_queue; queue_init(&halting_queue);

    RegisterAs(IO_SERVER_NAME);
    for (;;) {
        Receive(&tid, (char *)&request, sizeof(request));
        switch (request.type) {
            case IO_REQUEST_INT_UART:
                channel = iochannel(request.uart);
                io_server_disable_interrupts(request.uart, request.data);
                io_server_update_flag(channel, request.data);
                io_server_update_channel(channel);
                Reply(tid, NULL, 0);
                break;
            case IO_REQUEST_PUT:
                channel = iochannel(request.uart);
                if (queue_size(&channel->send_queue) > 0) {
                    queue_push(&channel->send_queue, tid);
                    break;
                }
                len = iochannel_buffer_put(channel, (char *)request.data, request.size);
                if (len > 0) {
                    Reply(tid, NULL, 0);
                } else {
                    queue_push(&channel->send_queue, tid);
                }
                uart_enable_interrupts(channel->uart, TIEN_MASK);
                break;
            case IO_REQUEST_GET:
                channel = iochannel(request.uart);
                if (queue_size(&channel->recv_queue) > 0) {
                    queue_push(&channel->recv_queue, tid);
                    break;
                }
                len = iochannel_buffer_get(channel, (char *)request.data, request.size);
                if (len > 0) {
                    Reply(tid, NULL, 0);
                } else {
                    queue_push(&channel->recv_queue, tid);
                    uart_enable_interrupts(channel->uart, RIEN_MASK);
                }
                break;
            case IO_REQUEST_SHUTDOWN:
                is_halting = true;
                halting_start = timer_read(TIMER3);
                if (iochannel_haltable(&com1_channel) && iochannel_haltable(&com2_channel)) {
                    Reply(tid, NULL, 0);
                } else {
                    queue_push(&halting_queue, tid);
                }
                break;
            default:
                break;
        }
        if (is_halting) {
            uint64_t now = timer_read(TIMER3);
            bool haltable = iochannel_haltable(&com1_channel) && iochannel_haltable(&com2_channel);
            if (now - halting_start > 1000 || haltable) { // ensure the IO server get halted in 1 seconds.
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
        uint32_t interrupts = (uint32_t) AwaitEvent(event);
        uart_clear_interrupts(request.uart);
        request.data = interrupts;
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
        uint32_t interrupts = (uint32_t) AwaitEvent(event);
        uart_clear_interrupts(request.uart);
        request.data = interrupts;
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
