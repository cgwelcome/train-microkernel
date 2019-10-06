#include <ts7200.h>

void icu_init() {
    volatile unsigned int *flag;
    flag = (unsigned int *) (VIC1_BASE + INTENABLE_OFFSET);
    *flag = 0x0;
    flag = (unsigned int *) (VIC2_BASE + INTENABLE_OFFSET);
    *flag = 0x0;
    flag = (unsigned int *) (VIC1_BASE + INTSELECT_OFFSET);
    *flag = 0x0;
    flag = (unsigned int *) (VIC2_BASE + INTSELECT_OFFSET);
    *flag = 0x0;
}

void icu_activate(unsigned int event) {
    volatile unsigned int *flag;
    unsigned int mask;

    if (event < 0 || event > 64) return;
    if (0 <= event && event < 32) {
        flag = (unsigned int *) (VIC1_BASE + INTENABLE_OFFSET);
        mask = 1 << event;
    } else {
        flag = (unsigned int *) (VIC2_BASE + INTENABLE_OFFSET);
        mask = 1 << (event-32);
    }
    *flag |= mask;
}

void icu_softirq(unsigned int event) {
    volatile unsigned int *flag;
    unsigned int mask;

    if (event < 0 || event > 64) return;
    if (0 <= event && event < 32) {
        flag = (unsigned int *) (VIC1_BASE + SOFTINT_OFFSET);
        mask = 1 << event;
    } else {
        flag = (unsigned int *) (VIC2_BASE + SOFTINT_OFFSET);
        mask = 1 << (event-32);
    }
    *flag |= mask;
}

int icu_activeirq(unsigned int event) {
    volatile unsigned int *flag;
    unsigned int mask;

    if (event < 0 || event > 64) return 0;
    if (0 <= event && event < 32) {
        flag = (unsigned int *) (VIC1_BASE + IRQSTATUS_OFFSET);
        mask = 1 << event;
    } else {
        flag = (unsigned int *) (VIC2_BASE + IRQSTATUS_OFFSET);
        mask = 1 << event;
    }
    return (*flag & mask) > 0 ? 1 : 0;
}

void icu_clear(unsigned int event) {
    volatile unsigned int *flag;
    unsigned int mask;

    if (event < 0 || event > 64) return;
    if (0 <= event && event < 32) {
        flag = (unsigned int *) (VIC1_BASE + INTENCLEAR_OFFSET);
        mask = 1 << event;
    } else {
        flag = (unsigned int *) (VIC2_BASE + INTENCLEAR_OFFSET);
        mask = 1 << (event-32);
    }
    *flag |= mask;
}
