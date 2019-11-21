#ifndef __USER_IO_H__
#define __USER_IO_H__

#include <stdarg.h>
#include <stddef.h>

/**
 * @defgroup io
 * @ingroup user
 *
 * @{
 */

/**
 * Returns next unreturned character from the given UART.
 * @return New character from the given UART.
 */
void Getc(int tid, int uart, char *c);

/**
 * Queues the given character for transmission by the given UART.
 */
void Putc(int tid, int uart, char c);

/**
 * Returns a fixed-size string from the given UART.
 */
void Getw(int tid, int uart, char *buffer, size_t size);

/**
 * Queues the given string for transmission by the given UART.
 */
void Putw(int tid, int uart, char *buffer, size_t size);

/**
 * Queues the given formatted string for transmission by the given UART.
 */
void Printf(int tid, int uart, char *fmt, ...);

#endif /*__USER_IO_H__*/
