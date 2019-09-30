#ifndef __UTILS_QUEUE_H__
#define __UTILS_QUEUE_H__

#define QUEUE_SIZE 2048

typedef struct {
    short array[QUEUE_SIZE];
    unsigned int head;
    unsigned int tail;
    unsigned int size;
} Queue;

void queue_init(Queue *queue);
short queue_pop(Queue *queue);
void queue_push(Queue *queue, short data);
unsigned int queue_size(Queue *queue);

#endif
