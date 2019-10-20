#include <server/trainset.h>
#include <utils/tsqueue.h>

void tsqueue_init(TSQueue *queue) {
    queue->head = 0;
    queue->tail = 0;
    queue->size = 0;
}

TSTimerResponse tsqueue_pop(TSQueue *queue) {
    TSTimerResponse data = queue->array[queue->head];
    queue->head = (queue->head + 1) % TSQUEUE_SIZE;
    queue->size--;
    return data;
}

void tsqueue_push(TSQueue *queue, TSTimerResponse response) {
    queue->array[queue->tail] = response;
    queue->tail = (queue->tail + 1) % TSQUEUE_SIZE;
    queue->size++;
}

uint32_t tsqueue_size(TSQueue *queue) {
    return queue->size;
}
