#include <hardware/icu.h>
#include <stdint.h>

void icu_init() {
    volatile uint32_t *flag;
    flag = (uint32_t *) (VIC1_BASE + INTENABLE_OFFSET);
    *flag = 0x0;
    flag = (uint32_t *) (VIC2_BASE + INTENABLE_OFFSET);
    *flag = 0x0;
    flag = (uint32_t *) (VIC1_BASE + INTSELECT_OFFSET);
    *flag = 0x0;
    flag = (uint32_t *) (VIC2_BASE + INTSELECT_OFFSET);
    *flag = 0x0;
}

void icu_activate(int event) {
    volatile uint32_t *flag;
    uint32_t mask;

    if (event >= 64) return;
    if (event < 32) {
        flag = (uint32_t *) (VIC1_BASE + INTENABLE_OFFSET);
        mask = 1U << event;
    } else {
        flag = (uint32_t *) (VIC2_BASE + INTENABLE_OFFSET);
        mask = 1U << (event-32);
    }
    *flag |= mask;
}

void icu_softirq(int event) {
    volatile uint32_t *flag;
    uint32_t mask;

    if (event >= 64) return;
    if (event < 32) {
        flag = (uint32_t *) (VIC1_BASE + SOFTINT_OFFSET);
        mask = 1U << event;
    } else {
        flag = (uint32_t *) (VIC2_BASE + SOFTINT_OFFSET);
        mask = 1U << (event-32);
    }
    *flag |= mask;
}

int icu_read() {
    volatile uint32_t *flag;

    flag = (uint32_t *) (VIC1_BASE + IRQSTATUS_OFFSET);
    if (*flag != 0) {
        return __builtin_ctz(*flag);
    }
    flag = (uint32_t *) (VIC2_BASE + IRQSTATUS_OFFSET);
    if (*flag != 0) {
        return 32 + __builtin_ctz(*flag);
    }
    return -1;
}

void icu_clear(int event) {
    volatile uint32_t *flag;
    uint32_t mask;

    if (event >= 64) return;
    if (event < 32) {
        flag = (uint32_t *) (VIC1_BASE + INTENCLEAR_OFFSET);
        mask = 1U << event;
    } else {
        flag = (uint32_t *) (VIC2_BASE + INTENCLEAR_OFFSET);
        mask = 1U << (event-32);
    }
    *flag |= mask;
}
