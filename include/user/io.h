#ifndef __USER_IO_H__
#define __USER_IO_H__

#include <stddef.h>

/**
 * @defgroup io
 * @ingroup user
 *
 * @{
 */

typedef char *va_list;

#define __va_argsiz(t)	\
		(((sizeof(t) + sizeof(int) - 1) / sizeof(int)) * sizeof(int))

#define va_start(ap, pN) ((ap) = ((va_list) __builtin_next_arg(pN)))

#define va_end(ap)	((void)0)

#define va_arg(ap, t)	\
		 (((ap) = (ap) + __va_argsiz(t)), *((t*) (void*) ((ap) - __va_argsiz(t))))

/**
 * Returns next unreturned character from the given UART.
 * @return New character from the given UART,
 * or -1 tid is not a valid uart server task,
 */
int Getc(int tid, int uart);

/**
 * Queues the given character for transmission by the given UART.
 * @return 0 sucess,
 * or -1 tid is not a valid uart server task.
 */
int Putc(int tid, int uart, char c);

/**
 * Returns a fixed-size string from the given UART.
 * @return 0 success, or -1 tid is not a valid uart server task.
 */
int Getw(int tid, int uart, char *buffer, size_t size);

/**
 * Queues the given string for transmission by the given UART.
 * @return 0 success, or -1 tid is not a valid uart server task.
 */
int Putw(int tid, int uart, char *buffer, size_t size);

/**
 * Queues the given formatted string
 * for transmission by the given UART.
 */
void Printf(int tid, int uart, char *fmt, ...);

#endif /*__USER_IO_H__*/
