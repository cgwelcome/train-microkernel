#include <kern/io.h>
#include <ts7200.h>

int setspeed(int channel, int speed) {
    int *high, *low;
    switch(channel) {
    case COM1:
        high = (int *)(UART1_BASE + UART_LCRM_OFFSET);
        low  = (int *)(UART1_BASE + UART_LCRL_OFFSET);
        break;
    case COM2:
        high = (int *)(UART2_BASE + UART_LCRM_OFFSET);
        low  = (int *)(UART2_BASE + UART_LCRL_OFFSET);
        break;
    default:
        return -1;
    }
    switch(speed) {
    case 115200:
        *high = 0x00;
        *low  = 0x03;
        return 0;
    case 2400:
        *high = 0x00;
        *low  = 0xC0;
        return 0;
    default:
        return -1;
    }
}

void io_init() {
    setspeed(COM1, 2400);
    int *com1_lcrh = (int *)(UART1_BASE + UART_LCRH_OFFSET);
    *com1_lcrh = WLEN_MASK | FEN_MASK | STP2_MASK;
    setspeed(COM2, 115200);
    int *com2_lcrh = (int *)(UART2_BASE + UART_LCRH_OFFSET);
    *com2_lcrh = WLEN_MASK;
}

int io_getc(int channel) {
    int *flags, *data;
    switch (channel) {
    case COM1:
        flags = (int *)(UART1_BASE + UART_FLAG_OFFSET);
        data  = (int *)(UART1_BASE + UART_DATA_OFFSET);
        break;
    case COM2:
        flags = (int *)(UART2_BASE + UART_FLAG_OFFSET);
        data  = (int *)(UART2_BASE + UART_DATA_OFFSET);
        break;
    default:
        return -1; // wrong channel, cannot continue
    }
    while ( *flags & RXFE_MASK ) ;
    return *data;
}

int io_putc(int channel, char c) {
    int *flags, *data;
	switch (channel) {
	case COM1:
		flags = (int *)( UART1_BASE + UART_FLAG_OFFSET );
		data  = (int *)( UART1_BASE + UART_DATA_OFFSET );
		break;
	case COM2:
		flags = (int *)( UART2_BASE + UART_FLAG_OFFSET );
		data  = (int *)( UART2_BASE + UART_DATA_OFFSET );
		break;
	default:
		return -1; // wrong channel, cannot continue
	}
	while( *flags & TXFF_MASK ) ;
	*data = c;
    return 0;
}
