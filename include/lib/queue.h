#ifndef __UTILS_QUEUE_H__
#define __UTILS_QUEUE_H__

#define QUEUE_SIZE 2000

typedef struct {
    int array[QUEUE_SIZE];
    int head;
    int tail;
    int size;
} Queue;

void q_init(Queue *q);
int q_pop(Queue *q);
void q_push(Queue *q, char data);
int q_size(Queue *q);

#endif
