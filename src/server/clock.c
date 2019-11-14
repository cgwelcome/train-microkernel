#include <kernel.h>
#include <hardware/timer.h>
#include <server/clock.h>
#include <user/event.h>
#include <user/ipc.h>
#include <user/name.h>
#include <user/tasks.h>
#include <utils/assert.h>
#include <utils/pqueue.h>

static PQueue delay_queue;
static PQueue schedule_queue;
static CSScheduleJob jobs[CLOCK_SCHEDULE_JOB_LIMIT];

static int clock_ticks() {
    return (int) (timer_read(TIMER3) / CLOCK_TICK_INTERVAL);
}

static void clock_time(int tid) {
    int now = clock_ticks();
    Reply(tid, (char *)&now, sizeof(now));
}

static int clock_schedule_next_job(CSScheduleJob *request) {
    for (int i = 0; i < CLOCK_SCHEDULE_JOB_LIMIT; i++) {
        if (jobs[i].in_use == 0) {
            jobs[i] = *request;
            jobs[i].in_use = 1;
            return i;
        }
    }
    throw("out of space for new schedule job");
}

static void clock_notify() {
    while (pqueue_size(&delay_queue) > 0 && pqueue_peek(&delay_queue) <= clock_ticks()) {
        int tid = pqueue_pop(&delay_queue);
        clock_time(tid);
    }
    while (pqueue_size(&schedule_queue) > 0 && pqueue_peek(&schedule_queue) <= clock_ticks()) {
        int jid = pqueue_pop(&schedule_queue);
        jobs[jid].in_use = 0;
        assert(Send(jobs[jid].target, jobs[jid].data, jobs[jid].data_size, NULL, 0) >= 0);
    }
}

static void clock_delay(int tid, int ticks) {
    if (ticks == 0) {
        clock_time(tid);
    } else {
        pqueue_insert(&delay_queue, tid, clock_ticks() + ticks);
    }
}

static void clock_delayuntil(int tid, int ticks) {
    if (ticks <= clock_ticks()) {
        clock_time(tid);
    } else {
        pqueue_insert(&delay_queue, tid, ticks);
    }
}

static void clock_schedule(int tid, CSScheduleJob * job) {
    if (job->ticks == 0) {
        assert(Send(job->target, job->data, job->data_size, NULL, 0) >= 0);
    } else {
        int job_id = clock_schedule_next_job(job);
        pqueue_insert(&schedule_queue, job_id, clock_ticks() + job->ticks);
    }
    Reply(tid, NULL, 0);
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
            case CS_SCHEDULE:
                clock_schedule(tid, (CSScheduleJob *) request.data);
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
    pqueue_init(&delay_queue);
    pqueue_init(&schedule_queue);
    for (size_t i = 0; i < CLOCK_SCHEDULE_JOB_LIMIT; i++) jobs[i].in_use = 0;
}

void CreateClockServer() {
    Create(PRIORITY_SERVER_CLOCK, &clock_server_task);
    Create(PRIORITY_NOTIFIER_CLOCK, &clock_notifier_task);
}
