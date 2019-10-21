#include <utils/queue.h>

void queue_init(Queue *queue) {
    queue->head = 0;
    queue->tail = 0;
    queue->size = 0;
}

int queue_pop(Queue *queue) {
    if (queue->size == 0) return -1;
    int data = queue->array[queue->head];
    queue->head = (queue->head + 1) % QUEUE_SIZE;
    queue->size--;
    return data;
}

void queue_push(Queue *queue, int data) {
    if (queue->size == QUEUE_SIZE) return;
    queue->array[queue->tail] = data;
    queue->tail = (queue->tail + 1) % QUEUE_SIZE;
    queue->size++;
}

uint32_t queue_size(Queue *queue) {
    return queue->size;
}

int queue_peek(Queue *queue) {
    return queue->array[0];
}
