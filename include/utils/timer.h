/*
 * timer.h
 */

#ifndef __UTILS_TIMER_H__
#define __UTILS_TIMER_H__

#define TIMER_FREQUENCY 508000
#define TIMER_FREQUENCY_MS 508

// timer_init initialize the 3rd timer for EP93XX processor.
void timer_init();

// timer_read() returns escaped time since bootup in milliseconds.
unsigned int timer_read();

// timer_read_raw() returns escaped time since bootup in ticks (508000 ticks / sec).
unsigned int timer_read_raw();

#endif // __UTILS_TIMER_H__