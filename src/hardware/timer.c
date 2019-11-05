#include <hardware/timer.h>
#include <utils/assert.h>

typedef struct {
    uint32_t load;
    int frequency;
} Timer;

static Timer timers[TIMER_MAXNUM];

void timer_init(int timer, uint32_t load, int frequency) {
    volatile uint32_t *load_reg = NULL;
    volatile uint32_t *ctrl_reg = NULL;
    switch (timer) {
    case TIMER1:
        load_reg = (uint32_t *)(TIMER1_BASE + LDR_OFFSET);
        ctrl_reg = (uint32_t *)(TIMER1_BASE + CRTL_OFFSET);
        break;
    case TIMER2:
        load_reg = (uint32_t *)(TIMER2_BASE + LDR_OFFSET);
        ctrl_reg = (uint32_t *)(TIMER2_BASE + CRTL_OFFSET);
        break;
    case TIMER3:
        load_reg = (uint32_t *)(TIMER3_BASE + LDR_OFFSET);
        ctrl_reg = (uint32_t *)(TIMER3_BASE + CRTL_OFFSET);
        break;
    default:
        panic("unknown timer", __FILE__, __LINE__);
    }

    *ctrl_reg = 0x0;
    *load_reg = load;
    *ctrl_reg = ENABLE_MASK | MODE_MASK;
    if (frequency == TIMER_HIGHFREQ) {
        *ctrl_reg |= CLKSEL_MASK;
    }

    timers[timer].load = load;
    timers[timer].frequency = frequency;
}

uint64_t timer_read(int timer) {
    assert(timer >= TIMER1 && timer <= TIMER3);
    assert(timers[timer].frequency != 0);
    return timer_read_raw(timer) / (uint64_t) timers[timer].frequency;
}

uint64_t timer_read_raw(int timer) {
    volatile uint32_t *time = NULL;
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
        panic("unknown timer", __FILE__, __LINE__);
    }
    return (uint64_t) timers[timer].load - (uint64_t) *time;
}

void timer_clear(int timer) {
    volatile uint32_t *flag = NULL;
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
        panic("unknown timer", __FILE__, __LINE__);
    }
    *flag = 1;
}
