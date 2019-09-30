#include <utils/queue.h>

void queue_init(Queue *queue) {
    queue->head = 0;
    queue->tail = 0;
    queue->size = 0;
}

short queue_pop(Queue *queue) {
    short data = queue->array[queue->head];
    queue->head = (queue->head + 1) % QUEUE_SIZE;
    queue->size--;
    return data;
}

void queue_push(Queue *queue, short data) {
    queue->array[queue->tail] = data;
    queue->tail = (queue->tail + 1) % QUEUE_SIZE;
    queue->size++;
}

unsigned int queue_size(Queue *queue) {
    return queue->size;
}
