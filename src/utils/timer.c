/*
 * timer.c - routines for reading EP93XX timer 3.
 *
 * Specific to the TS-7200 ARM evaluation board
 */

#include <ts7200.h>
#include <utils/timer.h>

void timer_init() {
  int *load = (int *)(TIMER3_BASE + LDR_OFFSET);
  int *ctrl = (int *)(TIMER3_BASE + CRTL_OFFSET);
  *ctrl = 0x00;
  *load = 0xFFFFFFFF;
  *ctrl = ENABLE_MASK | CLKSEL_MASK;
}

unsigned int timer_read() {
  return timer_read_raw() / TIMER_FREQUENCY;
}

unsigned int timer_read_raw() {
  unsigned int *time = (unsigned int *)(TIMER3_BASE + VAL_OFFSET);
  return (unsigned int)0xFFFFFFFF - *time;
}
