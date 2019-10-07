#ifndef __HARDWARE_TIMER_H__
#define __HARDWARE_TIMER_H__

#define TIMER_HIGHFREQ 508 // per millisecond
#define TIMER_LOWFREQ 2 // per millisecond

#define TIMER1 0
#define TIMER2 1
#define TIMER3 2

#define TIMER_MAXNUM 3
#define TIMER_MAXVAL 0xFFFFFFFF

#define	TIMER1_BASE	0x80810000
#define	TIMER2_BASE	0x80810020
#define	TIMER3_BASE	0x80810080

#define	LDR_OFFSET	0x00000000	// 16/32 bits, RW
#define	VAL_OFFSET	0x00000004	// 16/32 bits, RO
#define CRTL_OFFSET	0x00000008	// 3 bits, RW
	#define	ENABLE_MASK	0x00000080
	#define	MODE_MASK	0x00000040
	#define	CLKSEL_MASK	0x00000008
#define CLR_OFFSET	0x0000000c	// no data, WO

#include <stdint.h>

// timer_init initialize the a timers in the EP93XX processor.
void timer_init(int timer, int load, int frequency);

// timer_read() returns escaped time since bootup in milliseconds.
uint64_t timer_read(int timer);

// timer_read_raw() returns escaped time since bootup in ticks (508 ticks / millisecond).
uint64_t timer_read_raw(int timer);

// timer_clear() clears the timer interrupt 
void timer_clear(int timer);

#endif
