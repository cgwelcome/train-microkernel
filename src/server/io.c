#include <kernel.h>
#include <hardware/uart.h>
#include <hardware/timer.h>
#include <server/idle.h>
#include <server/io.h>
#include <user/event.h>
#include <user/ipc.h>
#include <user/name.h>
#include <user/tasks.h>
#include <utils/assert.h>
#include <utils/queue.h>

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
            throw("unknown uart");
    }
    uart_set_speed(uart, speed);
    uart_set_bitconfig(uart, bitconfig);
    uart_clean_buffer(uart);
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
            throw("unknown uart");
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
            throw("unknown uart");
    }
}

static bool iochannel_haltable(IOChannel *channel) {
    return queue_size(&channel->send_buffer) == 0;
}

static size_t iochannel_buffer_put(IOChannel *channel, char *buffer, size_t size) {
    assert(size <= QUEUE_SIZE);
    if (queue_size(&channel->send_buffer) + size <= QUEUE_SIZE) {
        for (size_t i = 0; i < size; i++) {
            queue_push(&channel->send_buffer, (int) buffer[i]);
        }
        return size;
    }
    return 0;
}

static size_t iochannel_buffer_get(IOChannel *channel, char *buffer, size_t size) {
    assert(size <= QUEUE_SIZE);
    if (queue_size(&channel->recv_buffer) >= size) {
        for (size_t i = 0; i < size; i++) {
            buffer[i] = (char) queue_pop(&channel->recv_buffer);
        }
        return size;
    }
    return 0;
}

static void iochannel_flush_send(IOChannel *channel) {
    // Flush send buffer
    int send_count = channel->fifo ? 8 : 1;
    for (int i = 0; i < send_count && queue_size(&channel->send_buffer) > 0; i++) {
        uart_putc(channel->uart, (char)queue_pop(&channel->send_buffer));
    }
    // Flush send queue
    IORequest request;
    while (queue_size(&channel->send_queue) > 0 && queue_size(&channel->send_buffer) < QUEUE_SIZE) {
        int tid = queue_peek(&channel->send_queue);
        Peek(tid, (char *)&request, sizeof(request));

        size_t len = iochannel_buffer_put(channel, (char *)request.data, request.size);
        if (len > 0) {
            queue_pop(&channel->send_queue);
            Reply(tid, NULL, 0);
        } else {
            break;
        }
    }
}

static void iochannel_flush_recv(IOChannel *channel) {
    // Flush recv buffer
    int recv_count = channel->fifo ? 8 : 1;
    for (int i = 0; i < recv_count && queue_size(&channel->recv_buffer) < QUEUE_SIZE; i++) {
        int c = uart_getc(channel->uart);
        if (c == -1) break;
        queue_push(&channel->recv_buffer, c);
    }
    // Flush recv queue
    IORequest request;
    while (queue_size(&channel->recv_queue) > 0 && queue_size(&channel->recv_buffer) != 0) {
        int tid = queue_peek(&channel->recv_queue);
        Peek(tid, (char *)&request, sizeof(request));

        size_t len = iochannel_buffer_get(channel, (char *)request.data, request.size);
        if (len > 0) {
            queue_pop(&channel->recv_queue);
            Reply(tid, NULL, 0);
        } else {
            break;
        }
    }
}

static void io_server_update_flag(IOChannel *channel, uint32_t flag) {
    if (channel->uart == COM1 && (flag & MIS_MASK)) {
        int uartstatus = uart_read_flags(channel->uart);
        if (!(uartstatus & CTS_MASK) && channel->cts_flag == IO_CTSINIT) {
            channel->cts_flag = IO_CTSDOWN;
        }
        else if (uartstatus & CTS_MASK && channel->cts_flag == IO_CTSDOWN) {
            channel->cts_flag = IO_CTSCOMPLETED;
            uart_disable_interrupts(channel->uart, MSIEN_MASK);
        }
    }
    if (channel->uart == COM2 && (flag & RTIS_MASK)) {
        channel->rx_flag = IO_RXDOWN;
        uart_disable_interrupts(channel->uart, RTIEN_MASK);
    }
    if (flag & RIS_MASK) {
        channel->rx_flag = IO_RXDOWN;
        uart_disable_interrupts(channel->uart, RIEN_MASK);
    }
    if (flag & TIS_MASK) {
        channel->tx_flag = IO_TXUP;
        uart_disable_interrupts(channel->uart, TIEN_MASK);
    }
}

static void io_server_update_channel(IOChannel *channel) {
    // Handle send logic
    if (channel->tx_flag == IO_TXUP && queue_size(&channel->send_buffer) > 0) {
        if (channel->uart == COM1 && channel->cts_flag == IO_CTSCOMPLETED) {
            iochannel_flush_send(channel);
            channel->tx_flag = IO_TXDOWN;
            channel->cts_flag = IO_CTSINIT;
            uart_enable_interrupts(channel->uart, TIEN_MASK | MSIEN_MASK);
        }
        if (channel->uart == COM2) {
            iochannel_flush_send(channel);
            channel->tx_flag = IO_TXDOWN;
            uart_enable_interrupts(channel->uart, TIEN_MASK);
        }
    }
    // Handle recv logic
    if (channel->rx_flag == IO_RXDOWN) {
        iochannel_flush_recv(channel);
        channel->rx_flag = IO_RXUP;
        if (channel->uart == COM1) {
            uart_enable_interrupts(channel->uart, RIEN_MASK);
        }
        if (channel->uart == COM2) {
            uart_enable_interrupts(channel->uart, RIEN_MASK | RTIEN_MASK);
        }
    }
}

void io_server_task() {
    int tid; IORequest request; IOChannel *channel; size_t len;
    bool is_halting = false; uint64_t halting_start = 0;
    Queue halting_queue; queue_init(&halting_queue);

    RegisterAs(SERVER_NAME_IO);
    io_init_uart(COM1);
    io_init_uart(COM2);
    io_init_channel(COM1);
    io_init_channel(COM2);
    for (;;) {
        Receive(&tid, (char *)&request, sizeof(request));
        switch (request.type) {
            case IO_REQUEST_INT_UART:
                channel = iochannel(request.uart);
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
                throw("unknown request");
        }
        if (is_halting) {
            uint64_t now = timer_read(TIMER3);
            bool haltable = iochannel_haltable(&com1_channel) && iochannel_haltable(&com2_channel);
            if (now - halting_start > 5000 || haltable) { // ensure the IO server get halted in 1 second.
                while (queue_size(&halting_queue)) {
                    int tid = queue_pop(&halting_queue);
                    Reply(tid, NULL, 0);
                }
                Exit();
            }
        }
    }
}

static void io_notifier_task(uint32_t uart) {
    assert(uart == COM1 || uart == COM2);

    int io_server_tid = WhoIs(SERVER_NAME_IO);
    int event = (uart == COM1 ? INT_UART1 : INT_UART2);
    IORequest request = {
        .type = IO_REQUEST_INT_UART,
        .uart = (int) uart,
    };
    for (;;) {
        uint32_t interrupts = (uint32_t) AwaitEvent(event);
        uart_clear_interrupts((int) uart);
        request.data = interrupts;
        assert(Send(io_server_tid, (char *)&request, sizeof(request), NULL, 0) >= 0);
    }
}

void CreateIOServer() {
    Create(PRIORITY_SERVER_IO, &io_server_task);
    CreateWithArg(PRIORITY_NOTIFIER_IO_COM1, &io_notifier_task, COM1);
    CreateWithArg(PRIORITY_NOTIFIER_IO_COM2, &io_notifier_task, COM2);
}

void ShutdownIOServer() {
    IORequest request = {
        .type = IO_REQUEST_SHUTDOWN
    };
    int io_server_tid = WhoIs(SERVER_NAME_IO);
    assert(Send(io_server_tid, (char *)&request, sizeof(request), NULL, 0) >= 0);
}
