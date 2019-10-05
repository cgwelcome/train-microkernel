#include <ts7200.h>
#include <event.h>
#include <application.h>
#include <user/tasks.h>
#include <user/irq.h>
#include <utils/icu.h>
#include <utils/bwio.h>
#include <utils/timer.h>

void await_test() {
    for (;;) {
        bwprintf(COM2, "Enter\n\r");
        AwaitEvent(TC2UI_EVENT);
        bwprintf(COM2, "Timer 2\n\r");
    }
}

void idle_test() {
    for (;;);
}

void irqtest_root_task() {
    Create(3000, &await_test);
    Create(1, &idle_test);
    Exit();
}
