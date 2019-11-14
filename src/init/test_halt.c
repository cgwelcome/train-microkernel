#include <hardware/syscon.h>
#include <hardware/timer.h>
#include <user/clock.h>
#include <utils/bwio.h>

void halt_test_root_task() {
    syscon_lock(OFF);
    syscon_config(SHENA_MASK);
    syscon_halt();
    bwprintf(COM2, "We shouldn't reach here!\r\n");
}
