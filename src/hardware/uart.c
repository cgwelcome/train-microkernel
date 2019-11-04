#include <stdint.h>
#include <hardware/uart.h>

int uart_set_bitconfig(int channel, uint8_t flag) {
    volatile uint8_t *line;
    switch (channel) {
        case COM1:
            line = (uint8_t *) (UART1_BASE + UART_LCRH_OFFSET);
            break;
        case COM2:
            line = (uint8_t *) (UART2_BASE + UART_LCRH_OFFSET);
            break;
        default:
            return -1;
            break;
    }
    *line = flag;
    return 0;
}

int uart_set_speed(int channel, int speed) {
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
        return -1;
        break;
    }
    switch( speed ) {
    case 115200:
        *high = 0x00;
        *low  = 0x03;
        return 0;
    case 2400:
        *high = 0x00;
        *low  = 0xBF;
        return 0;
    default:
        return -1;
    }
}

int uart_putc(int channel, uint8_t c) {
    volatile uint8_t *data;
    switch (channel) {
        case COM1:
            data  = (uint8_t *) (UART1_BASE + UART_DATA_OFFSET);
            break;
        case COM2:
            data  = (uint8_t *) (UART2_BASE + UART_DATA_OFFSET);
            break;
        default:
            return -1;
            break;
    }
    *data = c;
    return 0;
}

int uart_getc(int channel) {
    volatile uint8_t *flags, *data;
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
            return -1; // wrong channel, cannot continue
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
            return -1;
            break;
    }
    return *line;
}

int uart_enable_interrupts(int channel, uint8_t flag) {
    volatile uint8_t *line;
    switch (channel) {
        case COM1:
            line = (uint8_t *) (UART1_BASE + UART_CTLR_OFFSET);
            break;
        case COM2:
            line = (uint8_t *) (UART2_BASE + UART_CTLR_OFFSET);
            break;
        default:
            return -1;
            break;
    }
    *line = *line | flag;
    return 0;
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
            return -1;
            break;
    }
    return *line;
}

int uart_clear_interrupts(int channel) {
    volatile uint8_t *line;
    switch (channel) {
        case COM1:
            line = (uint8_t *) (UART1_BASE + UART_INTR_OFFSET);
            break;
        case COM2:
            line = (uint8_t *) (UART2_BASE + UART_INTR_OFFSET);
            break;
        default:
            return -1;
            break;
    }
    *line = 0;
    return 0;
}

int uart_disable_interrupts(int channel, uint8_t flag) {
    volatile uint8_t *line;
    switch (channel) {
        case COM1:
            line = (uint8_t *) (UART1_BASE + UART_CTLR_OFFSET);
            break;
        case COM2:
            line = (uint8_t *) (UART2_BASE + UART_CTLR_OFFSET);
            break;
        default:
            return -1;
            break;
    }
    *line = (uint8_t)(*line & ~flag);
    return 0;
}

int uart_disable_all_interrupts(int channel) {
    if (channel != COM1 && channel != COM2) return -1;
    uart_disable_interrupts(channel, MSIEN_MASK | TIEN_MASK | RIEN_MASK | RTIEN_MASK); return 0;
}
