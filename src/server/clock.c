#include <kernel.h>
#include <hardware/timer.h>
#include <server/clock.h>
#include <user/event.h>
#include <user/ipc.h>
#include <user/name.h>
#include <user/tasks.h>
#include <utils/assert.h>
#include <utils/pqueue.h>

static PQueue pqdelay;

static int clock_ticks() {
    return (int) (timer_read(TIMER3) / CLOCK_TICK_INTERVAL);
}

static void clock_time(int tid) {
    int now = clock_ticks();
    Reply(tid, (char *)&now, sizeof(now));
}

static void clock_notify() {
    while (pqueue_size(&pqdelay) > 0 && pqueue_peek(&pqdelay) <= clock_ticks()) {
        int tid = pqueue_pop(&pqdelay);
        clock_time(tid);
    }
}

static void clock_delay(int tid, int ticks) {
    if (ticks == 0) {
        clock_time(tid);
    } else {
        pqueue_insert(&pqdelay, tid, clock_ticks() + ticks);
    }
}

static void clock_delayuntil(int tid, int ticks) {
    if (ticks <= clock_ticks()) {
        clock_time(tid);
    } else {
        pqueue_insert(&pqdelay, tid, ticks);
    }
}

void clock_server_task() {
    int tid;
    CSRequest request;

    RegisterAs(SERVER_NAME_CLOCK);
    for (;;) {
        Receive(&tid, (char *)&request, sizeof(request));
        switch (request.type) {
            case CS_NOTIFY:
                Reply(tid, NULL, 0);
                clock_notify();
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
    int clock_server_tid = WhoIs(SERVER_NAME_CLOCK);
    timer_init(TIMER2, CLOCK_TICK_INTERVAL * TIMER_LOWFREQ, TIMER_LOWFREQ);
    CSRequest request = { .type = CS_NOTIFY };
    for (;;) {
        timer_clear(TIMER2);
        AwaitEvent(TC2UI_EVENT);
        assert(Send(clock_server_tid, (char *)&request, sizeof(request), NULL, 0) >= 0);
    }
}

void InitClockServer() {
    pqueue_init(&pqdelay);
}

void CreateClockServer() {
    Create(PRIORITY_SERVER_CLOCK, &clock_server_task);
    Create(PRIORITY_NOTIFIER_CLOCK, &clock_notifier_task);
}
