#include <stddef.h>
#include <ts7200.h>
#include <event.h>
#include <server/clock.h>
#include <user/ns.h>
#include <user/irq.h>
#include <user/ipc.h>
#include <user/tasks.h>
#include <utils/pqueue.h>
#include <utils/timer.h>
#include <utils/icu.h>

static PQueue pqdelay;
static int clockticks;

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
    }
    else {
        pqueue_insert(&pqdelay, tid, clockticks + ticks);
    }
}

static void cs_delayuntil(int tid, int ticks) {
    if (ticks <= clockticks) {
        cs_time(tid);
    }
    else {
        pqueue_insert(&pqdelay, tid, ticks);
    }
}

void cs_task() {
    int tid;
    CSRequest request;

    pqueue_init(&pqdelay);
    clockticks = 0;
    RegisterAs("CS");
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
    int cstid = WhoIs("CS");
    timer_init(TIMER2, TIMER_IRQ_INTERVAL * TIMER_LOWFREQ, TIMER_LOWFREQ);
    icu_activate(TC2UI_EVENT);
    CSRequest request = {
        .type = CS_TICKUPDATE
    };
    for (;;) {
        AwaitEvent(TC2UI_EVENT);
        Send(cstid, (char *)&request, sizeof(request), NULL, 0);
    }
}

int CreateCS(unsigned int priority) {
    int tid = Create(priority, &cs_task);
    Create(priority-1000, &cn_task);
    return tid;
}
