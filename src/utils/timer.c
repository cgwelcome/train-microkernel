/*
 * timer.c - routines for reading EP93XX timer 3.
 *
 * Specific to the TS-7200 ARM evaluation board
 */

#include <ts7200.h>
#include <utils/timer.h>

typedef struct {
    unsigned int load;
    int frequency;
} Timer;

static Timer timers[MAX_TIMER_NUM];

void timer_init(int timer, int load, int frequency) {
    volatile unsigned int *loadaddr;
    volatile unsigned int *ctrl;
    switch (timer) {
        case TIMER1:
            loadaddr = (unsigned int *)(TIMER1_BASE + LDR_OFFSET);
            ctrl = (unsigned int *)(TIMER1_BASE + CRTL_OFFSET);
            break;
        case TIMER2:
            loadaddr = (unsigned int *)(TIMER2_BASE + LDR_OFFSET);
            ctrl = (unsigned int *)(TIMER2_BASE + CRTL_OFFSET);
            break;
        case TIMER3:
            loadaddr = (unsigned int *)(TIMER3_BASE + LDR_OFFSET);
            ctrl = (unsigned int *)(TIMER3_BASE + CRTL_OFFSET);
            break;
        default:
            return;
            break;
    }
    *ctrl = 0x0;
    *loadaddr = load;
    *ctrl = ENABLE_MASK | MODE_MASK;
    if (frequency == TIMER_HIGHFREQ) {
        *ctrl |= CLKSEL_MASK;
    }
    timers[timer].load = load;
    timers[timer].frequency = TIMER_HIGHFREQ;
}

unsigned int timer_read(int timer) {
    return (timers[timer].load - timer_read_raw(timer)) / timers[timer].frequency;
}

unsigned int timer_read_raw(int timer) {
    volatile unsigned int *time;
    switch (timer) {
        case TIMER1:
            time = (unsigned int *)(TIMER1_BASE + VAL_OFFSET);
            break;
        case TIMER2:
            time = (unsigned int *)(TIMER2_BASE + VAL_OFFSET);
            break;
        case TIMER3:
            time = (unsigned int *)(TIMER3_BASE + VAL_OFFSET);
            break;
        default:
            return 0;
            break;
    }
    return *time;
}

void timer_clear(int timer) {
    volatile unsigned int *flag;
    switch (timer) {
        case TIMER1:
            flag = (unsigned int *)(TIMER1_BASE + CLR_OFFSET);
            break;
        case TIMER2:
            flag = (unsigned int *)(TIMER2_BASE + CLR_OFFSET);
            break;
        case TIMER3:
            flag = (unsigned int *)(TIMER3_BASE + CLR_OFFSET);
            break;
        default:
            return;
            break;
    }
    *flag = 1;
}
