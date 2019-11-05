#include <hardware/uart.h>
#include <utils/assert.h>

void uart_set_bitconfig(int channel, uint8_t flag) {
    volatile uint8_t *line = NULL;
    switch (channel) {
    case COM1:
        line = (uint8_t *) (UART1_BASE + UART_LCRH_OFFSET);
        break;
    case COM2:
        line = (uint8_t *) (UART2_BASE + UART_LCRH_OFFSET);
        break;
    default:
        panic("unknown uart", __FILE__, __LINE__);
    }
    *line = flag;
}

void uart_set_speed(int channel, int speed) {
    volatile uint8_t *high = NULL;
    volatile uint8_t *low  = NULL;
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
        panic("unknown uart", __FILE__, __LINE__);
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
        panic("unknown speed", __FILE__, __LINE__);
    }
}

void uart_putc(int channel, uint8_t c) {
    volatile uint8_t *data = NULL;
    switch (channel) {
    case COM1:
        data  = (uint8_t *) (UART1_BASE + UART_DATA_OFFSET);
        break;
    case COM2:
        data  = (uint8_t *) (UART2_BASE + UART_DATA_OFFSET);
        break;
    default:
        panic("unknown uart", __FILE__, __LINE__);
    }
    *data = c;
}

int uart_getc(int channel) {
    volatile uint8_t *flags = NULL;
    volatile uint8_t *data  = NULL;
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
        panic("unknown uart", __FILE__, __LINE__);
    }
    return *flags & RXFE_MASK ? -1 : *data;
}

int uart_read_flags(int channel) {
    volatile uint8_t *line = NULL;
    switch (channel) {
    case COM1:
        line = (uint8_t *) (UART1_BASE + UART_FLAG_OFFSET);
        break;
    case COM2:
        line = (uint8_t *) (UART2_BASE + UART_FLAG_OFFSET);
        break;
    default:
        panic("unknown uart", __FILE__, __LINE__);
    }
    return *line;
}

void uart_enable_interrupts(int channel, uint8_t flag) {
    volatile uint8_t *line = NULL;
    switch (channel) {
    case COM1:
        line = (uint8_t *) (UART1_BASE + UART_CTLR_OFFSET);
        break;
    case COM2:
        line = (uint8_t *) (UART2_BASE + UART_CTLR_OFFSET);
        break;
    default:
        panic("unknown uart", __FILE__, __LINE__);
    }
    *line = *line | flag;
}

int uart_read_interrupts(int channel) {
    volatile uint8_t *line = NULL;
    switch (channel) {
    case COM1:
        line = (uint8_t *) (UART1_BASE + UART_INTR_OFFSET);
        break;
    case COM2:
        line = (uint8_t *) (UART2_BASE + UART_INTR_OFFSET);
        break;
    default:
        panic("unknown uart", __FILE__, __LINE__);
    }
    return *line;
}

void uart_clear_interrupts(int channel) {
    volatile uint8_t *line = NULL;
    switch (channel) {
    case COM1:
        line = (uint8_t *) (UART1_BASE + UART_INTR_OFFSET);
        break;
    case COM2:
        line = (uint8_t *) (UART2_BASE + UART_INTR_OFFSET);
        break;
    default:
        panic("unknown uart", __FILE__, __LINE__);
    }
    *line = 0;
}

void uart_disable_interrupts(int channel, uint8_t flag) {
    volatile uint8_t *line = NULL;
    switch (channel) {
    case COM1:
        line = (uint8_t *) (UART1_BASE + UART_CTLR_OFFSET);
        break;
    case COM2:
        line = (uint8_t *) (UART2_BASE + UART_CTLR_OFFSET);
        break;
    default:
        panic("unknown uart", __FILE__, __LINE__);
    }
    *line = (uint8_t)(*line & ~flag);
}

void uart_disable_all_interrupts(int channel) {
    assert(channel == COM1 || channel == COM2);
    uart_disable_interrupts(channel, MSIEN_MASK | TIEN_MASK | RIEN_MASK | RTIEN_MASK);
}
