#ifndef __KERN_IO_H__
#define __KERN_IO_H__

#define COM1_FLUSH_INTERVAL 10 // ms
#define COM2_FLUSH_INTERVAL  0 // ms

void io_init();

int io_getc(int channel);

int io_putc(int channel, char c);

#endif // __KERN_IO_H__
