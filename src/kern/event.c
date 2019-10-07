#include <event.h>
#include <hardware/icu.h>
#include <hardware/timer.h>
#include <kern/tasks.h>
#include <utils/queue.h>

static Queue await_queue[MAX_EVENT_NUM];
static const int event_priority[] = {
    TC1UI_EVENT,
    TC2UI_EVENT,
    TC3UI_EVENT,
};

void event_init() {
    for (int i = 0; i < MAX_EVENT_NUM; i++) {
        queue_init(&await_queue[i]);
    }
}

void event_await(int tid, int eventid) {
    Task *current_task = task_at(tid);
    timer_clear(TIMER2);
    switch (eventid) {
        case TC1UI_EVENT:
        case TC2UI_EVENT:
        case TC3UI_EVENT:
            current_task->status = EVENTBLOCKED;
            queue_push(&await_queue[eventid], tid);
            break;
        default:
            current_task->tf->r0 = -1;
            break;
    }
}

void event_unblock(int eventid) {
    int returnvalue;
    switch (eventid) {
        case TC1UI_EVENT:
            timer_clear(TIMER1);
            returnvalue = 0;
            break;
        case TC2UI_EVENT:
            timer_clear(TIMER2);
            returnvalue = 0;
            break;
        case TC3UI_EVENT:
            timer_clear(TIMER3);
            returnvalue = 0;
            break;
        default:
            return;
            break;
    }
    while (queue_size(&await_queue[eventid]) > 0) {
        Task *task = task_at(queue_pop(&await_queue[eventid]));
        task->status = READY;
        task->tf->r0 = returnvalue;
    }
}

void event_handle() {
    // TODO: Make a sizeof Macro
    int size = sizeof(event_priority)/sizeof(event_priority[0]);

    for (int i = 0; i < size; i++) {
        if (icu_activeirq(event_priority[i])) {
            event_unblock(event_priority[i]);
            return;
        }
    }
}
