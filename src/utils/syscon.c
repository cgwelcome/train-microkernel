#include <ts7200.h>
#include <utils/syscon.h>

void syscon_lock(int state) {
    volatile unsigned int *flag = (unsigned int *)(SYSCON_BASE + SWLOCK_OFFSET);
    switch (state) {
        case ON:
            *flag = SWLOCK_ON;
            break;
        case OFF:
            *flag = SWLOCK_OFF;
            break;
        default:
            break;
    }
}

void syscon_config(unsigned int mask) {
    volatile unsigned int *flag = (unsigned int *)(SYSCON_BASE + DEVICE_CFG_OFFSET);
    *flag |= mask;
}
    

void syscon_halt() {
    *(volatile unsigned int *)(SYSCON_BASE + HALT_OFFSET);
}
