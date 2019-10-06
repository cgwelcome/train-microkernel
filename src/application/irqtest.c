#include <ts7200.h>
#include <event.h>
#include <application.h>
#include <server/idle.h>
#include <user/tasks.h>
#include <user/irq.h>
#include <utils/bwio.h>

void await_test() {
    for (;;) {
        bwprintf(COM2, "Enter\n\r");
        AwaitEvent(TC2UI_EVENT);
        bwprintf(COM2, "Exit\n\r");
    }
}

void irqtest_root_task() {
    Create(3000, &await_test);
    CreateIdle(1);
    Exit();
}
