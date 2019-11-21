/*
 * bwio.c - busy-wait I/O routines for diagnosis
 *
 * Specific to the TS-7200 ARM evaluation board
 *
 */

#include <utils/bwio.h>
#include <hardware/uart.h>

#include <stdio.h>

#define PRINTF_MAX_OUTPUT_SIZE 128

/*
 * The UARTs are initialized by RedBoot to the following state
 *  115,200 bps
 *  8 bits
 *  no parity
 *  fifos enabled
 */
int bwsetfifo( int channel, int state ) {
    volatile int *line, buf;
    switch( channel ) {
    case COM1:
        line = (int *)( UART1_BASE + UART_LCRH_OFFSET );
        break;
    case COM2:
        line = (int *)( UART2_BASE + UART_LCRH_OFFSET );
        break;
    default:
        return -1;
        break;
    }
    buf = *line;
    buf = state ? buf | FEN_MASK : buf & ~FEN_MASK;
    *line = buf;
    return 0;
}

int bwsetspeed( int channel, int speed ) {
    volatile int *high, *low;
    switch( channel ) {
    case COM1:
        high = (int *)( UART1_BASE + UART_LCRM_OFFSET );
        low  = (int *)( UART1_BASE + UART_LCRL_OFFSET );
        break;
    case COM2:
        high = (int *)( UART2_BASE + UART_LCRM_OFFSET );
        low  = (int *)( UART2_BASE + UART_LCRL_OFFSET );
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

int bwputc( int channel, char c ) {
    volatile int *flags, *data;
    switch( channel ) {
    case COM1:
        flags = (int *)( UART1_BASE + UART_FLAG_OFFSET );
        data  = (int *)( UART1_BASE + UART_DATA_OFFSET );
        break;
    case COM2:
        flags = (int *)( UART2_BASE + UART_FLAG_OFFSET );
        data  = (int *)( UART2_BASE + UART_DATA_OFFSET );
        break;
    default:
        return -1;
        break;
    }
    while( ( *flags & TXFF_MASK ) ) ;
    *data = c;
    while( ( *flags & TXFF_MASK ) ) ;
    return 0;
}

int bwgetc( int channel ) {
    volatile int *flags, *data;
    unsigned char c;

    switch( channel ) {
    case COM1:
        flags = (int *)( UART1_BASE + UART_FLAG_OFFSET );
        data = (int *)( UART1_BASE + UART_DATA_OFFSET );
        break;
    case COM2:
        flags = (int *)( UART2_BASE + UART_FLAG_OFFSET );
        data = (int *)( UART2_BASE + UART_DATA_OFFSET );
        break;
    default:
        return -1;
        break;
    }
    while ( !( *flags & RXFF_MASK ) ) ;
    c = (unsigned char) *data;
    while ( !( *flags & RXFF_MASK ) ) ;
    return c;
}

int bwputw( int channel, char *str ) {
    while( *str ) {
        if( bwputc( channel, *str ) < 0 ) return -1;
        str++;
    }
    return 0;
}

void bwprintf( int channel, char *fmt, ... ) {
    va_list va;

    va_start(va, fmt);
    char buffer[PRINTF_MAX_OUTPUT_SIZE];
    vsnprintf(buffer, PRINTF_MAX_OUTPUT_SIZE, fmt, va);
    bwputw(channel, buffer);
    va_end(va);
}
