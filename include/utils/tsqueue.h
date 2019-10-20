#ifndef __UTILS_TSQUEUE_H__
#define __UTILS_TSQUEUE_H__

#define TSQUEUE_SIZE 2048

#include <stdint.h>
#include <server/trainset.h>

// TODO: Make a Macro for generic type of queue
typedef struct {
    TSTimerResponse array[TSQUEUE_SIZE];
    uint32_t head;
    uint32_t tail;
    uint32_t size;
} TSQueue;

void tsqueue_init(TSQueue *queue);
TSTimerResponse tsqueue_pop (TSQueue *queue);
void tsqueue_push(TSQueue *queue, TSTimerResponse response);
uint32_t tsqueue_size(TSQueue *queue);

#endif
