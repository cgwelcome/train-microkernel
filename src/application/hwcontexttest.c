#include <ts7200.h>
#include <event.h>
#include <application.h>
#include <user/tasks.h>
#include <user/irq.h>
#include <utils/icu.h>
#include <utils/bwio.h>

void hwcontextswitchtest_root_task() {
    icu_softirq(TC2UI_EVENT);
    bwprintf(COM2, "He is back\n\r");
    Exit();
}
