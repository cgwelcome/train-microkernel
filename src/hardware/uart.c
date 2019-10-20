#include <stdint.h>
#include <hardware/uart.h>

int uart_setbitconfig(int channel, uint8_t flag) {
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

int uart_setspeed(int channel, int speed) {
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


int uart_enableintr(int channel, uint8_t flag) {
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

int uart_readintr(int channel) {
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

int uart_clearmsintr(int channel) {
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

int uart_readflag(int channel) {
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

int uart_disableintr(int channel, uint8_t flag) {
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

int uart_disableall(int channel) {
    if (channel != COM1 && channel != COM2) return -1;
    uart_disableintr(channel, TIEN_MASK);
    uart_disableintr(channel, RIEN_MASK); return 0;
}
