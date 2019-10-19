#include <stddef.h>
#include <event.h>
#include <hardware/timer.h>
#include <server/clock.h>
#include <user/event.h>
#include <user/ipc.h>
#include <user/name.h>
#include <user/tasks.h>
#include <utils/pqueue.h>

static int clock_server_tid, clock_notifier_tid;
static int clockticks;
static PQueue pqdelay;

static void cs_time(int tid) {
    Reply(tid, (char *)&clockticks, sizeof(clockticks));
}

static void cs_updatetick() {
    clockticks++;
    while (pqueue_size(&pqdelay) > 0 && pqueue_peek(&pqdelay) <= clockticks) {
        int tid = pqueue_pop(&pqdelay);
        cs_time(tid);
    }
}

static void cs_delay(int tid, int ticks) {
    if (ticks == 0) {
        cs_time(tid);
    } else {
        pqueue_insert(&pqdelay, tid, clockticks + ticks);
    }
}

static void cs_delayuntil(int tid, int ticks) {
    if (ticks <= clockticks) {
        cs_time(tid);
    } else {
        pqueue_insert(&pqdelay, tid, ticks);
    }
}

void cs_task() {
    int tid;
    CSRequest request;

    RegisterAs(CLOCK_SERVER_NAME);
    for (;;) {
        Receive(&tid, (char *)&request, sizeof(request));
        switch (request.type) {
            case CS_TICKUPDATE:
                cs_updatetick();
                Reply(tid, NULL, 0);
                break;
            case CS_TIME:
                cs_time(tid);
                break;
            case CS_DELAY:
                cs_delay(tid, request.data);
                break;
            case CS_DELAYUNTIL:
                cs_delayuntil(tid, request.data);
                break;
            default:
                break;
        }
    }
}

void cn_task() {
    int cstid = WhoIs(CLOCK_SERVER_NAME);
    timer_init(TIMER2, CLOCK_NOTIFY_INTERVAL * TIMER_LOWFREQ, TIMER_LOWFREQ);
    CSRequest request = {
        .type = CS_TICKUPDATE
    };
    for (;;) {
		timer_clear(TIMER2);
        AwaitEvent(TC2UI_EVENT);
        Send(cstid, (char *)&request, sizeof(request), NULL, 0);
    }
}

void InitClockServer() {
    clock_server_tid = -1;
    clock_notifier_tid = -1;
    clockticks = 0;
    pqueue_init(&pqdelay);
}

int CreateClockServer(uint32_t priority) {
    if (clock_server_tid < 0) {
        clock_server_tid = Create(priority, &cs_task);
    }
    if (clock_notifier_tid < 0) {
        clock_notifier_tid = Create(priority - 1000, &cn_task);
    }
    return clock_server_tid;
}
