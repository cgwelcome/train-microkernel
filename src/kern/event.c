#include <event.h>
#include <hardware/icu.h>
#include <hardware/timer.h>
#include <kern/tasks.h>
#include <utils/queue.h>

static Queue await_queue[MAX_EVENT_NUM];

void event_init() {
    for (int i = 0; i < MAX_EVENT_NUM; i++) {
        queue_init(&await_queue[i]);
    }
}

void event_await(int tid, int event) {
    Task *current_task = task_at(tid);
    timer_clear(TIMER2);
    switch (event) {
        case TC1UI_EVENT:
        case TC2UI_EVENT:
        case TC3UI_EVENT:
            current_task->status = EVENTBLOCKED;
            queue_push(&await_queue[event], tid);
            break;
        default:
            current_task->tf->r0 = (uint32_t) -1;
            break;
    }
}

void event_handle() {
    uint32_t return_value;
    int event = icu_read();
    switch (event) {
        case TC1UI_EVENT:
            timer_clear(TIMER1);
            return_value = 0;
            break;
        case TC2UI_EVENT:
            timer_clear(TIMER2);
            return_value = 0;
            break;
        case TC3UI_EVENT:
            timer_clear(TIMER3);
            return_value = 0;
            break;
        default:
            return;
    }
    while (queue_size(&await_queue[event]) > 0) {
        Task *task = task_at(queue_pop(&await_queue[event]));
        task->status = READY;
        task->tf->r0 = return_value;
    }
}
