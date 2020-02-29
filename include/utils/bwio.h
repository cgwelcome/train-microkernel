/*
 * bwio.h - provides busy-wait I/O operations.
 */

#ifndef __UTILS_BWIO_H__
#define __UTILS_BWIO_H__

#include <kernel.h>

#include <stdarg.h>

int bwsetfifo( int channel, int state );

int bwsetspeed( int channel, int speed );

int bwputc( int channel, char c );

int bwgetc( int channel );

int bwputw( int channel, char *str );

void bwprintf( int channel, char *format, ... );

#endif
