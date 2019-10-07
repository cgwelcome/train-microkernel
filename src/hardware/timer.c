#include <hardware/timer.h>

typedef struct {
    uint32_t load;
    int frequency;
} Timer;

static Timer timers[TIMER_MAXNUM];

void timer_init(int timer, uint32_t load, int frequency) {
    volatile uint32_t *loadaddr;
    volatile uint32_t *ctrl;
    switch (timer) {
        case TIMER1:
            loadaddr = (uint32_t *)(TIMER1_BASE + LDR_OFFSET);
            ctrl = (uint32_t *)(TIMER1_BASE + CRTL_OFFSET);
            break;
        case TIMER2:
            loadaddr = (uint32_t *)(TIMER2_BASE + LDR_OFFSET);
            ctrl = (uint32_t *)(TIMER2_BASE + CRTL_OFFSET);
            break;
        case TIMER3:
            loadaddr = (uint32_t *)(TIMER3_BASE + LDR_OFFSET);
            ctrl = (uint32_t *)(TIMER3_BASE + CRTL_OFFSET);
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

uint64_t timer_read(int timer) {
    return timer_read_raw(timer) / (uint64_t) timers[timer].frequency;
}

uint64_t timer_read_raw(int timer) {
    volatile uint32_t *time;
    switch (timer) {
        case TIMER1:
            time = (uint32_t *)(TIMER1_BASE + VAL_OFFSET);
            break;
        case TIMER2:
            time = (uint32_t *)(TIMER2_BASE + VAL_OFFSET);
            break;
        case TIMER3:
            time = (uint32_t *)(TIMER3_BASE + VAL_OFFSET);
            break;
        default:
            return 0;
            break;
    }
    return (uint64_t) timers[timer].load - (uint64_t) *time;
}

void timer_clear(int timer) {
    volatile uint32_t *flag;
    switch (timer) {
        case TIMER1:
            flag = (uint32_t *)(TIMER1_BASE + CLR_OFFSET);
            break;
        case TIMER2:
            flag = (uint32_t *)(TIMER2_BASE + CLR_OFFSET);
            break;
        case TIMER3:
            flag = (uint32_t *)(TIMER3_BASE + CLR_OFFSET);
            break;
        default:
            return;
            break;
    }
    *flag = 1;
}
