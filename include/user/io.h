#ifndef __USER_IO_H__
#define __USER_IO_H__

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

#endif
