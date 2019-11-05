#include <train/job.h>
#include <server/clock.h>
#include <server/trainmanager.h>
#include <user/clock.h>
#include <user/name.h>
#include <user/ipc.h>
#include <user/tasks.h>

void tjqueue_init(TrainJobQueue *queue) {
    queue->head = 0;
    queue->tail = 0;
    queue->size = 0;
}

TrainJob tjqueue_pop(TrainJobQueue *queue) {
    TrainJob job = queue->jobs[queue->head];
    queue->head = (queue->head + 1) % TJQUEUE_SIZE;
    queue->size--;
    return job;
}

void tjqueue_push(TrainJobQueue *queue, TrainJob *response) {
    queue->jobs[queue->tail] = *response;
    queue->tail = (queue->tail + 1) % TJQUEUE_SIZE;
    queue->size++;
}

uint32_t tjqueue_size(TrainJobQueue *queue) {
    return queue->size;
}

TrainJob create_trainjob(TMRequest request, uint32_t delay) {
     TrainJob job = {
         .request = request,
         .delay = delay,
     };
     return job;
}

void trainjob_notifier_task() {
    TrainJob job;
    TMRequest request = {
        .type = TMREQUESTTYPE_INIT_JOB,
    };
    int servertid = WhoIs(TRAINMANAGER_SERVER_NAME);
    int clocktid = WhoIs(CLOCK_SERVER_NAME);

    Send(servertid, (char *)&request, sizeof(request), (char *)&job, sizeof(job));
    Delay(clocktid, (int)job.delay);
    Send(servertid, (char *)&job.request, sizeof(job.request), NULL, 0);
    Exit();
}
