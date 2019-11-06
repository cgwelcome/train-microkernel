#include <hardware/icu.h>
#include <hardware/timer.h>
#include <hardware/uart.h>
#include <kern/event.h>
#include <kern/tasks.h>
#include <utils/assert.h>
#include <utils/queue.h>

static Queue await_queue[MAX_EVENT_NUM];

void event_init() {
    for (int i = 0; i < MAX_EVENT_NUM; i++) {
        queue_init(&await_queue[i]);
    }
}

void event_await(int tid, int event) {
    Task *current_task = task_at(tid);
    icu_activate(event);
    current_task->status = EVENTBLOCKED;
    queue_push(&await_queue[event], tid);
}

void event_handle() {
    uint32_t return_value;
    int event = icu_read();
    switch (event) {
        case TC1UI_EVENT:
        case TC2UI_EVENT:
        case TC3UI_EVENT:
            return_value = 0;
            break;
        case INT_UART1:
            return_value = (uint32_t) uart_read_interrupts(COM1);
            break;
        case INT_UART2:
            return_value = (uint32_t) uart_read_interrupts(COM2);
            break;
        default:
            throw("unsupported event");
    }
    icu_disable(event);
    while (queue_size(&await_queue[event]) > 0) {
        Task *task = task_at(queue_pop(&await_queue[event]));
        task->status = READY;
        task->tf->r0 = return_value;
    }
}
