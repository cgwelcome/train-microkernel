#ifndef __TRAIN_JOB_H__
#define __TRAIN_JOB_H__

#define TJQUEUE_SIZE 2048

#include <stdint.h>
#include <server/trainmanager.h>

typedef struct {
    TMRequest request;
    uint32_t delay;
} TrainJob;

typedef struct {
    TrainJob jobs[TJQUEUE_SIZE];
    uint32_t head;
    uint32_t tail;
    uint32_t size;
} TrainJobQueue;

void tjqueue_init(TrainJobQueue *queue);
TrainJob tjqueue_pop(TrainJobQueue *queue);
void tjqueue_push(TrainJobQueue *queue, TrainJob *response);
uint32_t tjqueue_size(TrainJobQueue *queue);

TrainJob create_trainjob(TMRequest request, uint32_t delay);
void trainjob_notifier_task();

#endif /*__TRAIN_JOB_H__*/
