#include <hardware/uart.h>
#include <utils/assert.h>

void uart_set_bitconfig(int channel, uint8_t flag) {
    volatile uint8_t *line;
    switch (channel) {
    case COM1:
        line = (uint8_t *) (UART1_BASE + UART_LCRH_OFFSET);
        break;
    case COM2:
        line = (uint8_t *) (UART2_BASE + UART_LCRH_OFFSET);
        break;
    default:
        throw("unknown uart");
    }
    *line = flag;
}

void uart_set_speed(int channel, int speed) {
    volatile uint8_t *high;
    volatile uint8_t *low;
    switch (channel) {
    case COM1:
        high = (uint8_t *) (UART1_BASE + UART_LCRM_OFFSET);
        low  = (uint8_t *) (UART1_BASE + UART_LCRL_OFFSET);
        break;
    case COM2:
        high = (uint8_t *) (UART2_BASE + UART_LCRM_OFFSET);
        low  = (uint8_t *) (UART2_BASE + UART_LCRL_OFFSET);
        break;
    default:
        throw("unknown uart");
    }
    switch( speed ) {
    case 115200:
        *high = 0x00;
        *low  = 0x03;
        break;
    case 2400:
        *high = 0x00;
        *low  = 0xBF;
        break;
    default:
        throw("unknown speed");
    }
}

void uart_clean_buffer(int channel) {
    for (int i = 0; i < 16; i++) uart_getc(channel);
}

void uart_putc(int channel, uint8_t c) {
    volatile uint8_t *data;
    switch (channel) {
    case COM1:
        data  = (uint8_t *) (UART1_BASE + UART_DATA_OFFSET);
        break;
    case COM2:
        data  = (uint8_t *) (UART2_BASE + UART_DATA_OFFSET);
        break;
    default:
        throw("unknown uart");
    }
    *data = c;
}

int uart_getc(int channel) {
    volatile uint8_t *flags;
    volatile uint8_t *data;
    switch(channel) {
    case COM1:
        flags = (uint8_t *)(UART1_BASE + UART_FLAG_OFFSET);
        data  = (uint8_t *)(UART1_BASE + UART_DATA_OFFSET);
        break;
    case COM2:
        flags = (uint8_t *)(UART2_BASE + UART_FLAG_OFFSET);
        data  = (uint8_t *)(UART2_BASE + UART_DATA_OFFSET);
        break;
    default:
        throw("unknown uart");
    }
    return *flags & RXFE_MASK ? -1 : *data;
}

int uart_read_flags(int channel) {
    volatile uint8_t *line;
    switch (channel) {
    case COM1:
        line = (uint8_t *) (UART1_BASE + UART_FLAG_OFFSET);
        break;
    case COM2:
        line = (uint8_t *) (UART2_BASE + UART_FLAG_OFFSET);
        break;
    default:
        throw("unknown uart");
    }
    return *line;
}

void uart_enable_interrupts(int channel, uint8_t flag) {
    volatile uint8_t *line;
    switch (channel) {
    case COM1:
        line = (uint8_t *) (UART1_BASE + UART_CTLR_OFFSET);
        break;
    case COM2:
        line = (uint8_t *) (UART2_BASE + UART_CTLR_OFFSET);
        break;
    default:
        throw("unknown uart");
    }
    *line = *line | flag;
}

int uart_read_interrupts(int channel) {
    volatile uint8_t *line;
    switch (channel) {
    case COM1:
        line = (uint8_t *) (UART1_BASE + UART_INTR_OFFSET);
        break;
    case COM2:
        line = (uint8_t *) (UART2_BASE + UART_INTR_OFFSET);
        break;
    default:
        throw("unknown uart");
    }
    return *line;
}

void uart_clear_interrupts(int channel) {
    volatile uint8_t *line;
    switch (channel) {
    case COM1:
        line = (uint8_t *) (UART1_BASE + UART_INTR_OFFSET);
        break;
    case COM2:
        line = (uint8_t *) (UART2_BASE + UART_INTR_OFFSET);
        break;
    default:
        throw("unknown uart");
    }
    *line = 0;
}

void uart_disable_interrupts(int channel, uint8_t flag) {
    volatile uint8_t *line;
    switch (channel) {
    case COM1:
        line = (uint8_t *) (UART1_BASE + UART_CTLR_OFFSET);
        break;
    case COM2:
        line = (uint8_t *) (UART2_BASE + UART_CTLR_OFFSET);
        break;
    default:
        throw("unknown uart");
    }
    *line = (uint8_t)(*line & ~flag);
}

void uart_disable_all_interrupts(int channel) {
    assert(channel == COM1 || channel == COM2);
    uart_disable_interrupts(channel, MSIEN_MASK | TIEN_MASK | RIEN_MASK | RTIEN_MASK);
}
