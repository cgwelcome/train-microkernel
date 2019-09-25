#include <utils/queue.h>

void q_init(Queue *q) {
    q->head = 0;
    q->tail = 0;
    q->size = 0;
}

int q_pop(Queue *q) {
    int data = q->array[q->head];
    q->head = (q->head + 1) % QUEUE_SIZE;
    q->size--;
    return data;
}

void q_push(Queue *q, char data) {
    q->array[q->tail] = data;
    q->tail = (q->tail + 1) % QUEUE_SIZE;
    q->size++;
}

int q_size(Queue *q) {
    return q->size;
}
