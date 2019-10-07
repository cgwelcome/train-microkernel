#include <hardware/timer.h>

typedef struct {
    unsigned int load;
    int frequency;
} Timer;

static Timer timers[TIMER_MAXNUM];

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

unsigned long timer_read(int timer) {
    return timer_read_raw(timer)/timers[timer].frequency;
}

unsigned long timer_read_raw(int timer) {
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
    return timers[timer].load - (unsigned long) *time;
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
