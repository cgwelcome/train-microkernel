#include <stddef.h>
#include <event.h>
#include <priority.h>
#include <hardware/timer.h>
#include <server/clock.h>
#include <user/event.h>
#include <user/ipc.h>
#include <user/name.h>
#include <user/tasks.h>
#include <utils/assert.h>
#include <utils/pqueue.h>

static int clock_server_tid, clock_notifier_tid;
static int clockticks;
static PQueue pqdelay;

static void clock_time(int tid) {
    Reply(tid, (char *)&clockticks, sizeof(clockticks));
}

static void clock_updatetick() {
    clockticks++;
    while (pqueue_size(&pqdelay) > 0 && pqueue_peek(&pqdelay) <= clockticks) {
        int tid = pqueue_pop(&pqdelay);
        clock_time(tid);
    }
}

static void clock_delay(int tid, int ticks) {
    if (ticks == 0) {
        clock_time(tid);
    } else {
        pqueue_insert(&pqdelay, tid, clockticks + ticks);
    }
}

static void clock_delayuntil(int tid, int ticks) {
    if (ticks <= clockticks) {
        clock_time(tid);
    } else {
        pqueue_insert(&pqdelay, tid, ticks);
    }
}

void clock_server_task() {
    int tid;
    CSRequest request;

    RegisterAs(CLOCK_SERVER_NAME);
    for (;;) {
        Receive(&tid, (char *)&request, sizeof(request));
        switch (request.type) {
            case CS_TICKUPDATE:
                clock_updatetick();
                Reply(tid, NULL, 0);
                break;
            case CS_TIME:
                clock_time(tid);
                break;
            case CS_DELAY:
                clock_delay(tid, request.data);
                break;
            case CS_DELAYUNTIL:
                clock_delayuntil(tid, request.data);
                break;
            default:
                throw("unknown request");
        }
    }
}

void clock_notifier_task() {
    timer_init(TIMER2, CLOCK_NOTIFY_INTERVAL * TIMER_LOWFREQ, TIMER_LOWFREQ);
    CSRequest request = {
        .type = CS_TICKUPDATE
    };
    for (;;) {
        timer_clear(TIMER2);
        AwaitEvent(TC2UI_EVENT);
        Send(clock_server_tid, (char *)&request, sizeof(request), NULL, 0);
    }
}

void InitClockServer() {
    clock_server_tid = -1;
    clock_notifier_tid = -1;
    clockticks = 0;
    pqueue_init(&pqdelay);
}

int CreateClockServer() {
    if (clock_server_tid < 0) {
        clock_server_tid = Create(PRIORITY_SERVER_CLOCK, &clock_server_task);
    }
    if (clock_notifier_tid < 0) {
        clock_notifier_tid = Create(PRIORITY_NOTIFIER_CLOCK, &clock_notifier_task);
    }
    return clock_server_tid;
}
