/*
 * timer.h
 */

#ifndef __UTILS_TIMER_H__
#define __UTILS_TIMER_H__

#define TIMER_HIGHFREQ 508 // per millisecond
#define TIMER_LOWFREQ 2 // per millisecond
#define MAX_TIMER_NUM 3

// timer_init initialize the a timers in the EP93XX processor.
void timer_init(int timer, int load, int frequency);

// timer_read() returns escaped time since bootup in milliseconds.
unsigned long timer_read(int timer);

// timer_read_raw() returns escaped time since bootup in ticks (508 ticks / millisecond).
unsigned long timer_read_raw(int timer);

// timer_clear() clears the timer interrupt 
void timer_clear(int timer);

#endif // __UTILS_TIMER_H__
