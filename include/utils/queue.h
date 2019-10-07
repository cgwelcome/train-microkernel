#ifndef __UTILS_QUEUE_H__
#define __UTILS_QUEUE_H__

#define QUEUE_SIZE 2048

#include <stdint.h>

typedef struct {
    int array[QUEUE_SIZE];
    uint32_t head;
    uint32_t tail;
    uint32_t size;
} Queue;

void queue_init(Queue *queue);
int  queue_pop (Queue *queue);
void queue_push(Queue *queue, int data);
uint32_t queue_size(Queue *queue);

#endif
