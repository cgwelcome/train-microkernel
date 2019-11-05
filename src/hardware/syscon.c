#include <hardware/syscon.h>
#include <utils/assert.h>

void syscon_lock(int state) {
    volatile uint32_t *flag = (uint32_t *)(SYSCON_BASE + SWLOCK_OFFSET);
    switch (state) {
    case ON:
        *flag = SWLOCK_ON;
        break;
    case OFF:
        *flag = SWLOCK_OFF;
        break;
    default:
        panic("unknown state", __FILE__, __LINE__);
    }
}

void syscon_config(uint32_t mask) {
    volatile uint32_t *flag = (uint32_t *)(SYSCON_BASE + DEVICE_CFG_OFFSET);
    *flag |= mask;
}


void syscon_halt() {
    *(volatile uint32_t *)(SYSCON_BASE + HALT_OFFSET);
}
