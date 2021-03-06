#ifndef __HARDWARE_UART_H__
#define __HARDWARE_UART_H__

#include <stdint.h>

/**
 * @defgroup uart
 * @ingroup hardware
 *
 * @{
 */

#define IRDA_BASE   0x808b0000
#define UART1_BASE  0x808c0000
#define UART2_BASE  0x808d0000

// All the below registers for UART1
// First nine registers (up to Ox28) for UART 2
#define UART_DATA_OFFSET    0x0     // low 8 bits
    #define DATA_MASK   0xff
#define UART_RSR_OFFSET     0x4     // low 4 bits
    #define FE_MASK     0x1
    #define PE_MASK     0x2
    #define BE_MASK     0x4
    #define OE_MASK     0x8
#define UART_LCRH_OFFSET    0x8     // low 7 bits
    #define BRK_MASK    0x1
    #define PEN_MASK    0x2     // parity enable
    #define EPS_MASK    0x4     // even parity
    #define STP2_MASK   0x8     // 2 stop bits
    #define FEN_MASK    0x10    // fifo
    #define WLEN_MASK   0x60    // word length
#define UART_LCRM_OFFSET    0xc     // low 8 bits
    #define BRDH_MASK   0xff    // MSB of baud rate divisor
#define UART_LCRL_OFFSET    0x10    // low 8 bits
    #define BRDL_MASK   0xff    // LSB of baud rate divisor
#define UART_CTLR_OFFSET    0x14    // low 8 bits
    #define UARTEN_MASK 0x1
    #define MSIEN_MASK  0x8     // modem status int
    #define RIEN_MASK   0x10    // receive int
    #define TIEN_MASK   0x20    // transmit int
    #define RTIEN_MASK  0x40    // receive timeout int
    #define LBEN_MASK   0x80    // loopback
#define UART_FLAG_OFFSET    0x18    // low 8 bits
    #define CTS_MASK    0x1
    #define DCD_MASK    0x2
    #define DSR_MASK    0x4
    #define TXBUSY_MASK 0x8
    #define RXFE_MASK   0x10    // Receive buffer empty
    #define TXFF_MASK   0x20    // Transmit buffer full
    #define RXFF_MASK   0x40    // Receive buffer full
    #define TXFE_MASK   0x80    // Transmit buffer empty
#define UART_INTR_OFFSET    0x1c
    #define MIS_MASK    0x1
    #define RIS_MASK    0x2
    #define TIS_MASK    0x4
    #define RTIS_MASK   0x8
#define UART_DMAR_OFFSET    0x28

// Specific to UART1
#define UART_MDMCTL_OFFSET  0x100
#define UART_MDMSTS_OFFSET  0x104
#define UART_HDLCCTL_OFFSET 0x20c
#define UART_HDLCAMV_OFFSET 0x210
#define UART_HDLCAM_OFFSET  0x214
#define UART_HDLCRIB_OFFSET 0x218
#define UART_HDLCSTS_OFFSET 0x21c

void uart_set_bitconfig(int channel, uint8_t buf);

void uart_set_speed(int channel, int speed);

void uart_clean_buffer(int channel);

void uart_putc(int channel, uint8_t c);

int uart_getc(int channel);

int uart_read_flags(int channel);

void uart_enable_interrupts(int channel, uint8_t flag);

int uart_read_interrupts(int channel);

void uart_clear_interrupts(int channel);

void uart_disable_interrupts(int channel, uint8_t flag);

void uart_disable_all_interrupts(int channel);

/** @} */

#endif
