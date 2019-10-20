#ifndef __USER_IO_H__
#define __USER_IO_H__

typedef char *va_list;

#define __va_argsiz(t)	\
		(((sizeof(t) + sizeof(int) - 1) / sizeof(int)) * sizeof(int))

#define va_start(ap, pN) ((ap) = ((va_list) __builtin_next_arg(pN)))

#define va_end(ap)	((void)0)

#define va_arg(ap, t)	\
		 (((ap) = (ap) + __va_argsiz(t)), *((t*) (void*) ((ap) - __va_argsiz(t))))

// Getc() returns next unreturned character from the given UART.
// Return Values:
//   >-1    new character from the given UART.
//    -1    tid is not a valid uart server task.
int Getc(int tid, int uart);

// Putc() queues the given character for transmission by the given UART.
// Return Values:
//    0     success
//   -1     tid is not a valid uart server task.
int Putc(int tid, int uart, char c);

void Putw(int tid, int uart, int n, char fc, char *bf);

void Printf(int tid, int uart, char *fmt, ...);

#endif
