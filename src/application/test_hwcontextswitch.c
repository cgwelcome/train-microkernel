#include <application.h>
#include <event.h>
#include <hardware/icu.h>
#include <user/tasks.h>
#include <utils/bwio.h>

void hw_context_switch_test_root_task() {
    icu_softirq(TC2UI_EVENT);
    bwprintf(COM2, "He is back\n\r");
    Exit();
}