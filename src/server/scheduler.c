#include <kernel.h>
#include <hardware/timer.h>
#include <server/scheduler.h>
#include <user/event.h>
#include <user/ipc.h>
#include <user/name.h>
#include <user/tasks.h>
#include <utils/assert.h>
#include <utils/pqueue.h>

static PQueue schedule_queue;
static SSRequest jobs[SCHEDULER_JOB_BUFFER_SIZE];

static int scheduler_ticks() {
    return (int) (timer_read(TIMER3) / SCHEDULER_TICK_INTERVAL);
}

static int scheduler_add_job(SSRequest *request) {
    for (int i = 0; i < SCHEDULER_JOB_BUFFER_SIZE; i++) {
        if (jobs[i].type == SS_DONE) {
            jobs[i] = *request;
            return i;
        }
    }
    throw("out of space for new scheduler job");
}

static void scheduler_notify() {
    while (pqueue_size(&schedule_queue) > 0 && pqueue_peek(&schedule_queue) <= scheduler_ticks()) {
        int jid = pqueue_pop(&schedule_queue);
        assert(Send(jobs[jid].target, jobs[jid].data, jobs[jid].data_size, NULL, 0) >= 0);
        jobs[jid].type = SS_DONE;
    }
}

static void scheduler_schedule(SSRequest *request) {
    if (request->ticks == 0) {
        assert(Send(request->target, request->data, request->data_size, NULL, 0) >= 0);
    } else {
        int job_id = scheduler_add_job(request);
        pqueue_insert(&schedule_queue, job_id, scheduler_ticks() + request->ticks);
    }
}

void scheduler_server_task() {
    int tid;
    SSRequest request;

    RegisterAs(SERVER_NAME_SCHEDULER);
    for (;;) {
        Receive(&tid, (char *)&request, sizeof(request));
        switch (request.type) {
            case SS_NOTIFY:
                Reply(tid, NULL, 0);
                scheduler_notify();
                break;
            case SS_SCHEDULE:
                Reply(tid, NULL, 0);
                scheduler_schedule(&request);
                break;
            default:
                throw("unknown request");
        }
    }
}

void scheduler_notifier_task() {
    int scheduler_server_tid = WhoIs(SERVER_NAME_SCHEDULER);
    timer_init(TIMER1, SCHEDULER_TICK_INTERVAL * TIMER_LOWFREQ, TIMER_LOWFREQ);
    SSRequest request = { .type = SS_NOTIFY };
    for (;;) {
        timer_clear(TIMER1);
        AwaitEvent(TC1UI_EVENT);
        assert(Send(scheduler_server_tid, (char *)&request, sizeof(request), NULL, 0) >= 0);
    }
}

void InitSchedulerServer() {
    pqueue_init(&schedule_queue);
    for (size_t i = 0; i < SCHEDULER_JOB_BUFFER_SIZE; i++) jobs[i].type = SS_DONE;
}

void CreateSchedulerServer() {
    Create(PRIORITY_SERVER_SCHEDULER, &scheduler_server_task);
    Create(PRIORITY_NOTIFIER_SCHEDULER, &scheduler_notifier_task);
}
