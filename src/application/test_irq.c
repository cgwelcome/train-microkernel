#include <application.h>
#include <event.h>
#include <server/idle.h>
#include <user/event.h>
#include <user/tasks.h>
#include <utils/bwio.h>

void await_test() {
    for (;;) {
        bwprintf(COM2, "Enter\n\r");
        AwaitEvent(TC2UI_EVENT);
        bwprintf(COM2, "Exit\n\r");
    }
}

void irq_test_root_task() {
    Create(3000, &await_test);
    CreateIdleTask(1);
    Exit();
}
